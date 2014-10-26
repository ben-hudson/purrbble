Pebble.addEventListener("appmessage", function(e) {
  width = 144 + Math.floor(Math.random() * 1024);
  height = 168 + Math.floor(Math.random() * 1024);

  url = "http://placekitten.com/" + width + "/" + height;
  convert(url, function(bytes) {
    Pebble.sendAppMessage({"receive": bytes});
  });
});

function convert(url, success) {
  url = "http://ec2-54-191-195-212.us-west-2.compute.amazonaws.com/api?image=" + url;
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
