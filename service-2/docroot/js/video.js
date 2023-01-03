var videoElement = document.querySelector('video');
var mobileCanvas = document.getElementById('mobileCanvas');




var mobileContext = mobileCanvas.getContext('2d');
var videoSelect = document.querySelector('select#videoSource');
var videoOption = document.getElementById('videoOption');
var buttonGo = document.getElementById('go');
var barcode_result = document.getElementById('dbr');
var test = document.getElementById('test');


	var imageWidth = 640, imageHeight = 480;
	mobileCanvas.width = imageWidth;
	mobileCanvas.height = imageHeight;

var ready = true;
var image = null;
var ZXing = null;
var decodePtr = null;
var Module = { onRuntimeInitialized: function () {
    ZXing = Module;
	image = ZXing._resize(imageWidth, imageHeight);
    decodePtr = ZXing.Runtime.addFunction(decodeCallback);
	scanBarcode();
  }
};

var decodeCallback = function (ptr, len, resultIndex, resultCount) {
  var result = new Uint8Array(ZXing.HEAPU8.buffer, ptr, len);
  console.log(resultCount);
  barcode_result.textContent = String.fromCharCode.apply(null, result);
    mobileCanvas.style.display = 'block';

  ready = true;
};

///0	NONE,
///1	AZTEC,
///2	CODABAR,
///3	CODE_39,
///4	CODE_93,
///5	CODE_128,
///6	DATA_MATRIX,
///7	EAN_8,
///8	EAN_13,
///9	ITF,
///10	MAXICODE,
///11	PDF_417,
///12	QR_CODE,
///13	RSS_14,
///14	RSS_EXPANDED,
///15	UPC_A,
///16	UPC_E,
///17	UPC_EAN_EXTENSION

// scan barcode
function scanBarcode() {
  barcode_result.textContent = ' ';

  if (ZXing == null) {
    alert("Barcode Reader is not ready!");
    return;
  }
  
  mobileContext.drawImage(videoElement, 0, 0, imageWidth, imageHeight);
  // read barcode
  var imageData = mobileContext.getImageData(0, 0, imageWidth, imageHeight);
  var idd = imageData.data;
//  var image = ZXing._resize(imageWidth, imageHeight);
  console.time("decode barcode");
  for (var i = 0, j = 0; i < idd.length; i += 4, j++) {
	  var brightness = (3*idd[i]+4*idd[i+1]+idd[i+2])>>>3;	
    ZXing.HEAPU8[image + j] = brightness;

//ZXing.HEAPU8[image + j] = idd[i];

  }
  var err = ZXing._decode_any(decodePtr);
  console.timeEnd('decode barcode');
  
  console.log("error code", err);
 // if (err == -2) {
    
 // }
  if (err == -2)
	  setTimeout(scanBarcode, 30);
  else
	  setTimeout(scanBarcode, 100);
}
// https://github.com/samdutton/simpl/tree/gh-pages/getusermedia/sources 
var videoSelect = document.querySelector('select#videoSource');

navigator.mediaDevices.enumerateDevices().then(gotDevices).then(getStream).catch(handleError);

videoSelect.onchange = getStream;

function gotDevices(deviceInfos) {
  for (var i = deviceInfos.length - 1; i >= 0; --i) {
    var deviceInfo = deviceInfos[i];
    var option = document.createElement('option');
    option.value = deviceInfo.deviceId;
    if (deviceInfo.kind === 'videoinput') {
      option.text = deviceInfo.label || 'camera ' + (videoSelect.length + 1);
      videoSelect.appendChild(option);
	  option.selected=true;
    } else {
      console.log('Found one other kind of source/device: ', deviceInfo);
    }
  }
}

function getStream() {
  buttonGo.disabled = false;
  if (window.stream) {
    window.stream.getTracks().forEach(function(track) {
      track.stop();
    });
  }

  var constraints = {
    video: {
		frameRate: 10,
      deviceId: {exact: videoSelect.value}, height: imageHeight, width:imageWidth
    }
  };

navigator.mediaDevices.getUserMedia(constraints).then(gotStream).catch(handleError);


}

function gotStream(stream) {
  window.stream = stream; // make stream available to console
  videoElement.srcObject = stream;
  
 // const track = stream.getVideoTracks()[0];
	//		const { width, height } = track.getSettings();
	//		mobileCanvas.width = width;
	//		mobileCanvas.height = height;
			
	//		imageWidth = width; imageHeight = height;
			
}

function handleError(error) {
  console.log('Error: ', error);
}