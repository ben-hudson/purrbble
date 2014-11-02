Pebble.addEventListener("ready", function(e) {
  search("cat");
});

Pebble.addEventListener("appmessage", function(e) {
  if(localStorage.shake == "on") {
    search("cat");
  }
});

Pebble.addEventListener("showConfiguration", function(e) {
  Pebble.openURL("http://ben-hudson.github.io/defaced?" + encodeURIComponent(JSON.stringify(localStorage)));
});

Pebble.addEventListener("webviewclosed", function(e) {
  if(e.response != "CANCELLED" && e.response != "{}") {
    var settings = JSON.parse(decodeURIComponent(e.response));
    localStorage.shake = settings.shake;
  } else if(JSON.stringify(localStorage) == "{}") {
    localStorage.shake = "off";
  }
});

function search(term) {
  console.log("searching for " + term);

  var page = Math.floor(Math.random() * 500);
  var url = "https://api.datamarket.azure.com/Bing/Search/v1/Image?Query=%27" + encodeURIComponent(term) + "%27&$skip=" + page + "&$format=json";
  var Base64={_keyStr:"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",encode:function(e){var t="";var n,r,i,s,o,u,a;var f=0;e=Base64._utf8_encode(e);while(f<e.length){n=e.charCodeAt(f++);r=e.charCodeAt(f++);i=e.charCodeAt(f++);s=n>>2;o=(n&3)<<4|r>>4;u=(r&15)<<2|i>>6;a=i&63;if(isNaN(r)){u=a=64}else if(isNaN(i)){a=64}t=t+this._keyStr.charAt(s)+this._keyStr.charAt(o)+this._keyStr.charAt(u)+this._keyStr.charAt(a)}return t},decode:function(e){var t="";var n,r,i;var s,o,u,a;var f=0;e=e.replace(/[^A-Za-z0-9\+\/\=]/g,"");while(f<e.length){s=this._keyStr.indexOf(e.charAt(f++));o=this._keyStr.indexOf(e.charAt(f++));u=this._keyStr.indexOf(e.charAt(f++));a=this._keyStr.indexOf(e.charAt(f++));n=s<<2|o>>4;r=(o&15)<<4|u>>2;i=(u&3)<<6|a;t=t+String.fromCharCode(n);if(u!=64){t=t+String.fromCharCode(r)}if(a!=64){t=t+String.fromCharCode(i)}}t=Base64._utf8_decode(t);return t},_utf8_encode:function(e){e=e.replace(/\r\n/g,"\n");var t="";for(var n=0;n<e.length;n++){var r=e.charCodeAt(n);if(r<128){t+=String.fromCharCode(r)}else if(r>127&&r<2048){t+=String.fromCharCode(r>>6|192);t+=String.fromCharCode(r&63|128)}else{t+=String.fromCharCode(r>>12|224);t+=String.fromCharCode(r>>6&63|128);t+=String.fromCharCode(r&63|128)}}return t},_utf8_decode:function(e){var t="";var n=0;var r=c1=c2=0;while(n<e.length){r=e.charCodeAt(n);if(r<128){t+=String.fromCharCode(r);n++}else if(r>191&&r<224){c2=e.charCodeAt(n+1);t+=String.fromCharCode((r&31)<<6|c2&63);n+=2}else{c2=e.charCodeAt(n+1);c3=e.charCodeAt(n+2);t+=String.fromCharCode((r&15)<<12|(c2&63)<<6|c3&63);n+=3}}return t}};
  var auth = Base64.encode(":ZsxhClcFdjO267av1f5l+7JTQvUHNI4lfg3t4PO1L98");

  var request = new XMLHttpRequest();
  request.open("GET", url, true);
  request.setRequestHeader("Authorization", "Basic " + auth);
  request.onload = function(e) {
    var response = JSON.parse(request.response);
    if(request.status == 200 && response) {
      results = response.d.results
      url = results[Math.floor(Math.random() * results.length)].MediaUrl;
      convert(url);
    }
  }
  request.send(null);
}

function convert(image) {
  console.log("converting " + image);

  var url = "http://ec2-54-191-195-212.us-west-2.compute.amazonaws.com/api?size=144x128&image=" + image;

  var request = new XMLHttpRequest();
  request.open("GET", url, true);
  request.responseType = "arraybuffer";
  request.onload = function(e) {
    var response = request.response;
    if(request.status == 200 && response) {
      var bytes = new Uint8Array(response);
      var array = [];
      for(var i = 0; i < bytes.byteLength; i++) {
        array.push(bytes[i]);
      }
      send(array, 1600);
    }
  }
  request.send(null);
}

function send(bytes, chunkSize) {
  sendChunk = function(start) {
    console.log("sending " + bytes.length + " bytes - starting at " + start);

    var chunk = bytes.slice(start, start + chunkSize);
    Pebble.sendAppMessage({"incoming": chunk}, function(e) {
      if(bytes.length > start + chunkSize) {
        sendChunk(start + chunkSize);
      }
    });
  }
  sendChunk(0);
}
