/* Isolated SSE serialization shared by general-assistant regressions. */
static char *general_read_wire(const char *capability)
{
    yyjson_mut_doc *d=yyjson_mut_doc_new(NULL);yyjson_mut_val *args=yyjson_mut_obj(d);
    yyjson_mut_doc_set_root(d,args);yyjson_mut_obj_add_strcpy(d,args,"target_alias","target-0");
    yyjson_mut_obj_add_strcpy(d,args,"capability_id",capability);yyjson_mut_obj_add_strcpy(d,args,"parameters_json","{}");
    char *raw=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);if(!raw)return NULL;
    d=yyjson_mut_doc_new(NULL);yyjson_mut_doc_set_root(d,yyjson_mut_strcpy(d,raw));free(raw);
    char *quoted=yyjson_mut_write(d,0,NULL);yyjson_mut_doc_free(d);if(!quoted)return NULL;
    size_t capacity=strlen(quoted)*4+3000;char *wire=malloc(capacity);
    if(wire)snprintf(wire,capacity,
      "event: response.created\r\ndata: {\"type\":\"response.created\",\"sequence_number\":0,\"response\":{\"id\":\"prep\"}}\r\n\r\n"
      "event: response.output_item.added\r\ndata: {\"type\":\"response.output_item.added\",\"sequence_number\":1,\"item\":{\"type\":\"function_call\",\"id\":\"item-0\",\"call_id\":\"prep-call\",\"name\":\"xs_read_call\",\"arguments\":\"\"}}\r\n\r\n"
      "event: response.function_call_arguments.delta\r\ndata: {\"type\":\"response.function_call_arguments.delta\",\"sequence_number\":2,\"item_id\":\"item-0\",\"delta\":%s}\r\n\r\n"
      "event: response.function_call_arguments.done\r\ndata: {\"type\":\"response.function_call_arguments.done\",\"sequence_number\":3,\"item_id\":\"item-0\",\"arguments\":%s}\r\n\r\n"
      "event: response.output_item.done\r\ndata: {\"type\":\"response.output_item.done\",\"sequence_number\":4,\"item\":{\"type\":\"function_call\",\"id\":\"item-0\",\"call_id\":\"prep-call\",\"name\":\"xs_read_call\",\"arguments\":%s}}\r\n\r\n"
      "event: response.completed\r\ndata: {\"type\":\"response.completed\",\"sequence_number\":5,\"response\":{\"id\":\"prep\",\"status\":\"completed\",\"output\":[{\"type\":\"function_call\",\"id\":\"item-0\",\"call_id\":\"prep-call\",\"name\":\"xs_read_call\",\"arguments\":%s}],\"usage\":{\"input_tokens\":10,\"output_tokens\":4}}}\r\n\r\n",quoted,quoted,quoted,quoted);
    free(quoted);return wire;
}
static char *general_wire(const char *text,int missing_usage)
{
    yyjson_mut_doc *quoted_doc=yyjson_mut_doc_new(NULL);
    yyjson_mut_doc_set_root(quoted_doc,yyjson_mut_strcpy(quoted_doc,text));
    char *quoted=yyjson_mut_write(quoted_doc,0,NULL);yyjson_mut_doc_free(quoted_doc);CHECK(quoted!=NULL);if(!quoted)return NULL;
    size_t size=strlen(quoted)*2+2048;char *wire=malloc(size);CHECK(wire!=NULL);
    if(wire)snprintf(wire,size,
      "event: response.created\r\ndata: {\"type\":\"response.created\",\"sequence_number\":0,\"response\":{\"id\":\"general-fixture\"}}\r\n\r\n"
      "event: response.output_text.delta\r\ndata: {\"type\":\"response.output_text.delta\",\"sequence_number\":1,\"delta\":%s}\r\n\r\n"
      "event: response.completed\r\ndata: {\"type\":\"response.completed\",\"sequence_number\":2,\"response\":{\"id\":\"general-fixture\",\"status\":\"completed\",\"output\":[{\"type\":\"message\",\"role\":\"assistant\",\"content\":[{\"type\":\"output_text\",\"text\":%s}]}]%s}}\r\n\r\n",
      quoted,quoted,missing_usage?"":",\"usage\":{\"input_tokens\":10,\"output_tokens\":4}");
    free(quoted);return wire;
}

