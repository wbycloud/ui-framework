use deepseek_recipe::openai::ResponsesRequest;
use deepseek_recipe::request::{ConversionOptions, ModelVersion, ProtocolRequest};
use deepseek_recipe_encoding::v4::dsv41::DeepseekV41Encoding;
use deepseek_recipe_encoding::{PromptEncoding, PromptTokenizer};
use serde_json::{Value, json};
use sha2::{Digest, Sha256};
use std::io::{self, Read};
use std::mem::size_of;
use tokenizers::Tokenizer;
use windows_sys::Win32::Foundation::{CloseHandle, HANDLE};
use windows_sys::Win32::System::Diagnostics::Debug::{
    SEM_FAILCRITICALERRORS, SEM_NOGPFAULTERRORBOX, SetErrorMode,
};
use windows_sys::Win32::System::JobObjects::{
    AssignProcessToJobObject, CreateJobObjectW, JOB_OBJECT_LIMIT_PROCESS_MEMORY,
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION, JobObjectExtendedLimitInformation,
    SetInformationJobObject,
};
use windows_sys::Win32::System::ProcessStatus::{K32GetProcessMemoryInfo, PROCESS_MEMORY_COUNTERS};
use windows_sys::Win32::System::Threading::GetCurrentProcess;

const REQUEST_LIMIT: u64 = 128 * 1024;
const OUTPUT_LIMIT: u64 = 4096;
const COMMIT_LIMIT: usize = 128 * 1024 * 1024;
const PROFILE: &str = "deepseek-v41-responses-1017-v1";

struct MemoryJob(HANDLE);

impl Drop for MemoryJob {
    fn drop(&mut self) {
        unsafe { CloseHandle(self.0) };
    }
}

fn memory_job() -> Result<MemoryJob, &'static str> {
    unsafe {
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
        let handle = CreateJobObjectW(std::ptr::null(), std::ptr::null());
        if handle.is_null() {
            return Err("meter_job_create");
        }
        let job = MemoryJob(handle);
        let mut limits: JOBOBJECT_EXTENDED_LIMIT_INFORMATION = std::mem::zeroed();
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY;
        limits.ProcessMemoryLimit = COMMIT_LIMIT;
        if SetInformationJobObject(
            handle,
            JobObjectExtendedLimitInformation,
            &limits as *const _ as *const _,
            size_of::<JOBOBJECT_EXTENDED_LIMIT_INFORMATION>() as u32,
        ) == 0
            || AssignProcessToJobObject(handle, GetCurrentProcess()) == 0
        {
            return Err("meter_job_limit");
        }
        Ok(job)
    }
}

fn text_request(value: &Value) -> bool {
    let Some(root) = value.as_object() else {
        return false;
    };
    if root.len() != 7
        || root.keys().any(|key| !["model", "stream", "max_output_tokens", "reasoning", "instructions", "input", "tools"].contains(&key.as_str()))
        || value["reasoning"].as_object().map(|object| object.len()) != Some(1)
    {
        return false;
    }
    if value["max_output_tokens"].as_u64() != Some(OUTPUT_LIMIT)
        || value["reasoning"]["effort"].as_str() != Some("none")
        || value["stream"].as_bool() != Some(true)
        || !value["instructions"].is_string()
    {
        return false;
    }
    let Some(items) = value["input"].as_array() else {
        return false;
    };
    let Some(tools) = value["tools"].as_array() else {
        return false;
    };
    tools.iter().all(|tool| tool["type"].as_str() == Some("function"))
        && items.iter().all(|item| match item["type"].as_str() {
            None | Some("message") => item["content"].is_string(),
            Some("function_call") => item["arguments"].is_string(),
            Some("function_call_output") => item["output"].is_string(),
            _ => false,
        })
}

fn measure() -> Result<Value, &'static str> {
    let _job = memory_job()?;
    tokenizers::utils::parallelism::set_parallelism(false);
    let mut body = Vec::new();
    io::stdin()
        .take(REQUEST_LIMIT + 1)
        .read_to_end(&mut body)
        .map_err(|_| "meter_input_read")?;
    if body.is_empty() || body.len() as u64 > REQUEST_LIMIT {
        return Err("meter_input_limit");
    }
    let value: Value = serde_json::from_slice(&body).map_err(|_| "meter_json")?;
    let model = value["model"].as_str().ok_or("meter_model")?;
    if model != "deepseek-flash" && !(cfg!(feature = "fixture") && model == "offline-fixture") {
        return Err("meter_model");
    }
    if !text_request(&value) {
        return Err("meter_protocol");
    }
    let request: ResponsesRequest = serde_json::from_value(value).map_err(|_| "meter_schema")?;
    let options = ConversionOptions::default()
        .with_model_alias("deepseek-flash", ModelVersion::DeepseekV41Flash)
        .with_model_alias("offline-fixture", ModelVersion::DeepseekV41Flash);
    let converted = request.convert(options).map_err(|_| "meter_conversion")?;
    if converted.conversation.thinking_mode {
        return Err("meter_thinking");
    }
    let tokenizer = Tokenizer::from_bytes(include_bytes!("../tokenizer.json"))
        .map_err(|_| "meter_tokenizer")?;
    let encoding = DeepseekV41Encoding::new().with_tokenizer(PromptTokenizer::new(&tokenizer));
    let tokens = encoding
        .encode(&converted.conversation)
        .map_err(|_| "meter_encoding")?;
    if tokens.is_empty() || tokens.len() > 1_048_576 {
        return Err("meter_context");
    }
    let output = converted
        .resolve_max_tokens(tokens.len() as u32, 1_048_576)
        .map_err(|_| "meter_context")?;
    if output != OUTPUT_LIMIT as u32 {
        return Err("meter_output_limit");
    }
    let mut memory: PROCESS_MEMORY_COUNTERS = unsafe { std::mem::zeroed() };
    memory.cb = size_of::<PROCESS_MEMORY_COUNTERS>() as u32;
    let observed = unsafe { K32GetProcessMemoryInfo(GetCurrentProcess(), &mut memory, memory.cb) };
    if observed == 0 || memory.PeakPagefileUsage > COMMIT_LIMIT {
        return Err("meter_memory_observation");
    }
    Ok(json!({
        "ok": true,
        "profile": PROFILE,
        "evidence": format!("{:x}", Sha256::digest(include_bytes!("../profile.json"))),
        "body_hash": format!("{:x}", Sha256::digest(&body)),
        "input_tokens": tokens.len(),
        "output_limit": output,
        "peak_commit_bytes": memory.PeakPagefileUsage,
    }))
}

fn main() {
    std::panic::set_hook(Box::new(|_| {}));
    let measured = std::panic::catch_unwind(measure).unwrap_or(Err("meter_panic"));
    let ok = measured.is_ok();
    let result = measured.unwrap_or_else(|error| json!({"ok": false, "error": error}));
    let encoded = serde_json::to_string(&result).unwrap_or_else(|_| "{\"ok\":false}".into());
    if encoded.len() >= 1024 {
        println!("{{\"ok\":false,\"error\":\"meter_receipt_limit\"}}");
        std::process::exit(2);
    }
    println!("{encoded}");
    std::process::exit(if ok { 0 } else { 2 });
}
