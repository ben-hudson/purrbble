Pebble.addEventListener("appmessage", function(e) {
  console.log("Received request for cat...");

  search("cat");
});

function search(term) {
  var page = Math.floor(Math.random() * 500);
  var url = "https://api.datamarket.azure.com/Bing/Search/v1/Image?Query=%27cats%27&$skip=" + page + "&$format=json";
  var request = new XMLHttpRequest();
  var auth = window.btoa(":ZsxhClcFdjO267av1f5l+7JTQvUHNI4lfg3t4PO1L98")
  request.open("GET", url, true);   
  request.setRequestHeader("Authorization", "Basic " + auth); 
  request.onload = function(e) {
    response = JSON.parse(request.response);
    url = response.d.results[0].MediaUrl;
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
