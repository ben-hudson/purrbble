Pebble.addEventListener("appmessage", function(e) {
  console.log("Received request for cat...");

  search("cat");
});

function search(term) {
  var page = Math.floor(Math.random() * 500);
  var Base64={_keyStr:"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",encode:function(e){var t="";var n,r,i,s,o,u,a;var f=0;e=Base64._utf8_encode(e);while(f<e.length){n=e.charCodeAt(f++);r=e.charCodeAt(f++);i=e.charCodeAt(f++);s=n>>2;o=(n&3)<<4|r>>4;u=(r&15)<<2|i>>6;a=i&63;if(isNaN(r)){u=a=64}else if(isNaN(i)){a=64}t=t+this._keyStr.charAt(s)+this._keyStr.charAt(o)+this._keyStr.charAt(u)+this._keyStr.charAt(a)}return t},decode:function(e){var t="";var n,r,i;var s,o,u,a;var f=0;e=e.replace(/[^A-Za-z0-9\+\/\=]/g,"");while(f<e.length){s=this._keyStr.indexOf(e.charAt(f++));o=this._keyStr.indexOf(e.charAt(f++));u=this._keyStr.indexOf(e.charAt(f++));a=this._keyStr.indexOf(e.charAt(f++));n=s<<2|o>>4;r=(o&15)<<4|u>>2;i=(u&3)<<6|a;t=t+String.fromCharCode(n);if(u!=64){t=t+String.fromCharCode(r)}if(a!=64){t=t+String.fromCharCode(i)}}t=Base64._utf8_decode(t);return t},_utf8_encode:function(e){e=e.replace(/\r\n/g,"\n");var t="";for(var n=0;n<e.length;n++){var r=e.charCodeAt(n);if(r<128){t+=String.fromCharCode(r)}else if(r>127&&r<2048){t+=String.fromCharCode(r>>6|192);t+=String.fromCharCode(r&63|128)}else{t+=String.fromCharCode(r>>12|224);t+=String.fromCharCode(r>>6&63|128);t+=String.fromCharCode(r&63|128)}}return t},_utf8_decode:function(e){var t="";var n=0;var r=c1=c2=0;while(n<e.length){r=e.charCodeAt(n);if(r<128){t+=String.fromCharCode(r);n++}else if(r>191&&r<224){c2=e.charCodeAt(n+1);t+=String.fromCharCode((r&31)<<6|c2&63);n+=2}else{c2=e.charCodeAt(n+1);c3=e.charCodeAt(n+2);t+=String.fromCharCode((r&15)<<12|(c2&63)<<6|c3&63);n+=3}}return t}};
  var url = "https://api.datamarket.azure.com/Bing/Search/v1/Image?Query=%27cats%27&$skip=" + page + "&$format=json";
  var request = new XMLHttpRequest();
  var auth = Base64.encode(":ZsxhClcFdjO267av1f5l+7JTQvUHNI4lfg3t4PO1L98");
  request.open("GET", url, true);   
  request.setRequestHeader("Authorization", "Basic " + auth); 
  request.onload = function(e) {
    response = JSON.parse(request.response);
    results = response.d.results
    result = Math.floor(Math.random() * results.length);
    console.log("Getting result " + result + " of page " + page);
    url = results[result].MediaUrl;
    convert(url, function(e) {
      send(e, 2000);
    })
  }
  request.send(null);  
}

Pebble.addEventListener("ready", function(e) {
  console.log("Received request for cat...");

  search("cat");
});

function send(bytes, chunk_size) {
send_chunk = function(start) {
  var chunk = bytes.slice(start, start + chunk_size);
  console.log("Sending " + chunk.length + " bytes - starting at offset " + start);
  Pebble.sendAppMessage({ "receive": chunk }, function(e) {
    if (bytes.length > start + chunkSize) {
      send_chunk(start + chunk_size);
    }
  }, function (e) {
    if (retries++ < 3) {
      console.log("Got a nack for chunk #" + start + " - retry...");
      send_chunk(start);
    }
  });
};

send_chunk(0);
}

function convert(url, success) {
  url = "http://ec2-54-191-195-212.us-west-2.compute.amazonaws.com/api?size=144x128&image=" + url;
  console.log("Getting " + url);
  var request = new XMLHttpRequest();
  request.open("GET", url, true);
  request.responseType = "arraybuffer";
  request.onload = function(e) {
    var buffer = request.response;
    if(request.status == 200 && buffer) {
      var bytes = new Uint8Array(buffer);
      var array = [];
      for(var i = 0; i < bytes.byteLength; i++) {
        array.push(bytes[i]);
      }
      success(array);
    }
  }
  request.send(null);
}
