Pebble.addEventListener("ready", function(e) {
  search("cat");
});

Pebble.addEventListener("appmessage", function(e) {
  if (localStorage.shake == "on") {
    search("cat");
  }
});

Pebble.addEventListener("showConfiguration", function(e) {
  Pebble.openURL("http://ben-hudson.github.io/purrbble?" + encodeURIComponent(JSON.stringify(localStorage)));
});

Pebble.addEventListener("webviewclosed", function(e) {
  if (e.response != "CANCELLED" && e.response != "{}") {
    var settings = JSON.parse(decodeURIComponent(e.response));
    localStorage.shake = settings.shake;
  }
});

function search(term) {
  console.log("searching for " + term);

  var key = "4283f0a08ac557ff26c25ab8361f08a7";
  var page = Math.floor(Math.random() * 1024);
  var url = "https://api.flickr.com/services/rest/?method=flickr.photos.search&api_key=" + key + "&text=" + term + "&sort=relevance&per_page=1&page=" + page + "&format=json&nojsoncallback=1";

  var request = new XMLHttpRequest();
  request.open("GET", url, true);
  request.onload = function(e) {
    var response = JSON.parse(request.response);
    if (request.status == 200 && response) {
      var result = response.photos.photo[0];
      var photo = "https://farm" + result.farm + ".staticflickr.com/" + result.server + "/" + result.id + "_" + result.secret + ".jpg";
      convert(photo);
    }
  }
  request.send(null);
}

var sending = false;

function convert(photo) {
  console.log("converting " + photo);

  var url = "http://remote-magick.herokuapp.com/api?size=144x168&image=" + photo;

  var request = new XMLHttpRequest();
  request.open("GET", url, true);
  request.responseType = "arraybuffer";
  request.onload = function(e) {
    var response = request.response;
    if (request.status == 200 && response) {
      var bytes = new Uint8Array(response);
      var array = [];
      for (var i = 0; i < bytes.byteLength; i++) {
        array.push(bytes[i]);
      }
      if (!sending) {
        sending = true;
        send(array, 1000);
      } else {
        console.log("sending another image is in progress");
      }
    }
  }
  request.send(null);
}

function send(bytes, chunkSize) {
  var retries = 0;
  sendChunk = function(start) {
    var end = start + chunkSize;
    var chunk = bytes.slice(start, end);

    console.log("sending bytes " + Math.min(end, bytes.length) + "/" + bytes.length);

    Pebble.sendAppMessage({
      "data": chunk
    }, function(e) {
      if (bytes.length > end) {
        sendChunk(end);
      } else {
        sending = false;
        retries = 0;
      }
    }, function(e) {
      if (retries++ < 3) {
        sendChunk(start);
      } else {
        console.log("giving up sending chunk");
        sending = false;
        retries = 0;
      }
    });
  }
  Pebble.sendAppMessage({
    "size": bytes.length
  });
  sendChunk(0);
}
