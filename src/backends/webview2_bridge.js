if (window === window.top) {
 window.ui = {
  invoke: function(command, params) {
   if(typeof command !== 'string' || !/^[A-Za-z0-9_.-]{1,255}$/.test(command)) throw new TypeError('invalid command');
   var json = typeof params === 'string' ? params : JSON.stringify(params || {});
   JSON.parse(json); chrome.webview.postMessage('ui.invoke\n' + command + '\n' + json);
  },
  value: function(id) {var e=document.getElementById(id);return e?e.value:'';},
  postMessage: function(value) {if(value&&value.action==='metrics'){value.nodes=String(document.querySelectorAll('*').length);var rows=document.getElementById('rows');value.rows_height=String(rows?rows.getBoundingClientRect().height:0);}var json=typeof value==='string'?value:JSON.stringify(value);JSON.parse(json);chrome.webview.postMessage('ui.json\n'+json);}
 };
 chrome.webview.addEventListener('message', function(event) {
  var packet=event.data,data=packet.__ui_payload;
  if(typeof ui.onmessage==='function')ui.onmessage(data);
  window.dispatchEvent(new MessageEvent('message',{data:data}));
  chrome.webview.postMessage('ui.rendered\n'+packet.__ui_epoch);
 });
 function images(root) {
  if(!root || !root.querySelectorAll)return;
  var all=Array.from(root.querySelectorAll('img'));if(root.tagName==='IMG')all.push(root);
  all.forEach(function(e){var s=e.getAttribute('src');if(/^[0-9]+$/.test(s||'')){
   e.dataset.uiImage=s;if(s==='0'){e.removeAttribute('src');return;}
   e.src='https://ui.framework.invalid/image/'+s+'?v='+Date.now();
  }});
 }
 ui.refreshImage=function(id,version){document.querySelectorAll('img').forEach(function(e){if(e.dataset.uiImage===id)e.src='https://ui.framework.invalid/image/'+id+'?v='+version;});};
 new MutationObserver(function(list){list.forEach(function(m){if(m.type==='attributes')images(m.target);else m.addedNodes.forEach(images);});}).observe(document,{subtree:true,childList:true,attributes:true,attributeFilter:['src']});
 ui.capture=function(token){Promise.all(Array.from(document.images).map(function(e){return e.decode().catch(function(){});})).then(function(){return document.fonts.ready;}).then(function(){requestAnimationFrame(function(){requestAnimationFrame(function(){chrome.webview.postMessage('ui.capture\n'+token);});});});};
 ui.dispatchInput=function(p){
  var e=p.kind<=4?document.elementFromPoint(p.x,p.y):document.activeElement;
  var edit=e&&(e.tagName==='INPUT'||e.tagName==='TEXTAREA')&&!e.disabled&&!e.readOnly;
  var options={bubbles:true,cancelable:true,ctrlKey:!!(p.mods&1),shiftKey:!!(p.mods&2),altKey:!!(p.mods&4),keyCode:p.key,which:p.key};
  if(e&&!e.disabled){
   var mouse={bubbles:true,cancelable:true,clientX:p.x,clientY:p.y,button:p.button===2?2:0,ctrlKey:options.ctrlKey,shiftKey:options.shiftKey,altKey:options.altKey};
   if(p.kind===2){var focus=e.closest('input,textarea,button,select,[tabindex]');if(focus)focus.focus();ui.pressed=e;e.dispatchEvent(new MouseEvent('mousedown',mouse));}
   if(p.kind===3){e.dispatchEvent(new MouseEvent('mouseup',mouse));if(e===ui.pressed){if(p.button===2)e.dispatchEvent(new MouseEvent('contextmenu',mouse));else e.click();}ui.pressed=null;}
   if(p.kind===1)e.dispatchEvent(new MouseEvent('mousemove',{bubbles:true,clientX:p.x,clientY:p.y}));
   if(p.kind===4){var wheel=new WheelEvent('wheel',{bubbles:true,cancelable:true,deltaY:-p.delta,shiftKey:!!(p.mods&2)});if(e.dispatchEvent(wheel)){for(var n=e;n;n=n.parentElement)if(n.scrollHeight>n.clientHeight||n.scrollWidth>n.clientWidth){if(p.mods&2)n.scrollLeft-=p.delta;else n.scrollTop-=p.delta;break;}}}
   if(p.kind===7&&edit)document.execCommand('insertText',false,p.text);
   if(p.kind===5&&e.dispatchEvent(new KeyboardEvent('keydown',options))){
    chrome.webview.postMessage('ui.shortcut\n'+p.key+' '+p.mods+' '+(edit?1:0));
    if(p.key===13||p.key===32){if(!edit)e.click();}
    else if(p.key===9){var all=Array.from(document.querySelectorAll('button,input,textarea,select,[tabindex]')).filter(function(n){return !n.disabled&&n.getClientRects().length;});if(all.length)all[(all.indexOf(e)+all.length+((p.mods&2)?-1:1))%all.length].focus();}
    else if(edit){var a=e.selectionStart,b=e.selectionEnd;if(p.mods&1){if(p.key===65)e.select();if(p.key===90)document.execCommand('undo');if(p.key===89)document.execCommand('redo');}
     else if(p.key===8||p.key===46)document.execCommand(p.key===8?'delete':'forwardDelete');
     else if(p.key===37||p.key===39||p.key===36||p.key===35){var at=p.key===36?0:p.key===35?e.value.length:p.key===37?Math.max(0,a-1):Math.min(e.value.length,b+1);e.setSelectionRange((p.mods&2)?a:at,at);}
    }
   }
   if(p.kind===6)e.dispatchEvent(new KeyboardEvent('keyup',options));
  }
  chrome.webview.postMessage('ui.rendered\n'+p.epoch);
 };
}
