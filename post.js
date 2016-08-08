	var API = {};
	function _simple(image, opt){
		// for converting canvas image data into pnm format
		if(image.data){
			var width     = image.width, height = image.height, src = image.data;
			var header    = "P5\n" + width + " " + height + "\n255\n";
			var j         = header.length;
			var dst       = new Uint8Array(j + width * height);
			var srcLength = src.length | 0, srcLength_16 = (srcLength - 16) | 0;
			
			for(var i = 0; i < j; i++) dst[i] = header.charCodeAt(i); // write the header
			var coeff_r = 4899, coeff_g = 9617, coeff_b = 1868;

			for (var i = 0; i <= srcLength_16; i += 16, j += 4) { // convert to grayscale 4 pixels at a time
				dst[j]     = (src[i] * coeff_r + src[i+1] * coeff_g + src[i+2] * coeff_b + 8192) >> 14;
				dst[j + 1] = (src[i+4] * coeff_r + src[i+5] * coeff_g + src[i+6] * coeff_b + 8192) >> 14;
				dst[j + 2] = (src[i+8] * coeff_r + src[i+9] * coeff_g + src[i+10] * coeff_b + 8192) >> 14;
				dst[j + 3] = (src[i+12] * coeff_r + src[i+13] * coeff_g + src[i+14] * coeff_b + 8192) >> 14;
			}
			for (; i < srcLength; i += 4, ++j)
				dst[j] = (src[i] * coeff_r + src[i+1] * coeff_g + src[i+2] * coeff_b + 8192) >> 14;
			image = dst;
		}
		// for pnm buffers
		if(image instanceof ArrayBuffer) image = new Uint8Array(image);
		API.write_file('/in.pnm', image);

		var desc = API.open();
		API.set_image_from_file(desc, '/in.pnm', opt.invert ? 1 : 0);

		if(opt.raw) API.set_exportfile(desc, '/out.txt');

		if(opt.filters){
			(opt.filters.forEach ? opt.filters : [opt.filters]).forEach(function(filter_name){
				if(["letters", "letters_only", "numbers", "numbers_only", "same_height", "upper_num", "upper_num_only"].indexOf(filter_name) != -1){
					API.add_filter(desc, filter_name);	
				}else throw "Invalid Filter";
			})
		}
		if(opt.transform){
	 		if(["none", "rotate90", "rotate180", "rotate270", "mirror_lr", "mirror_tb", "mirror_d1", "mirror_d2"].indexOf(opt.transform) != -1){
	 			API.transform(desc, opt.transform);
	 		}else throw "Invalid transformation!";
		}
		if(opt.scale){
			if(API.scale(desc, Math.round(opt.scale)) < 0)
	      throw "Error scaling image";
		}
		API.set_utf8_format(desc, 1);
		API.recognize(desc, 0);

		var ret;
		if(opt.raw){
			ret = API.read_text('/out.txt');
			API.delete_file('/out.txt');
		}else{
			var text = '';
			var block_count = API.result_blocks(desc);
			
			for(var i = 0; i < block_count; i++){
				var line_count = API.result_lines(desc, i);
				for(var j = 0; j < line_count; j++){
					var line = API.result_line(desc, i, j);
					text += line;
				}
			}
			ret = text;
		}
		API.close(desc);
		API.delete_file('/in.pnm');
		return ret;
	}

	// BEGIN API SECTION //
	API.set_print              = function(fn) { Module.print = fn };
	API.write_file             = function(filename, arr){ FS.writeFile(filename, arr, {encoding: 'binary'}) };
	API.read_file              = function(filename){ return FS.readFile(filename, {encoding: 'binary'}) };
	API.read_text              = function(filename){ return FS.readFile(filename, {encoding: 'utf8'}) };
	API.delete_file            = function(filename){ return FS.unlink(filename) };
	API.version                = Module.cwrap('OCRAD_version', 'string');
	API.open                   = Module.cwrap('OCRAD_open', 'number');
	API.close                  = Module.cwrap('OCRAD_close', 'number', ['number']);
	API.get_errno              = Module.cwrap('OCRAD_get_errno', 'number', ['number']);
	API.set_image              = Module.cwrap('OCRAD_set_image', 'number', ['number', 'number', 'number']);
	API.set_image_from_file    = Module.cwrap('OCRAD_set_image_from_file', 'number', ['number', 'string', 'number']);
	API.set_exportfile         = Module.cwrap('OCRAD_set_exportfile', 'number', ['number', 'string']);
	API.add_filter             = Module.cwrap('OCRAD_add_filter', 'number', ['number', 'string']);
	API.set_utf8_format        = Module.cwrap('OCRAD_set_utf8_format', 'number', ['number', 'number']);
	API.set_threshold          = Module.cwrap('OCRAD_set_threshold', 'number', ['number', 'number']);
	API.scale                  = Module.cwrap('OCRAD_scale', 'number', ['number', 'number']);
	API.transform              = Module.cwrap('OCRAD_transform', 'number', ['number', 'string']);
	API.recognize              = Module.cwrap('OCRAD_recognize', 'number', ['number', 'number']);
	API.result_blocks          = Module.cwrap('OCRAD_result_blocks', 'number', ['number']);
	API.result_lines           = Module.cwrap('OCRAD_result_lines', 'number', ['number', 'number']);
	API.result_chars_total     = Module.cwrap('OCRAD_result_chars_total', 'number', ['number']);
	API.result_chars_block     = Module.cwrap('OCRAD_result_chars_block', 'number', ['number', 'number']);
	API.result_chars_line      = Module.cwrap('OCRAD_result_chars_line', 'number', ['number', 'number', 'number']);
	API.result_line            = Module.cwrap('OCRAD_result_line', 'string', ['number', 'number', 'number']);
	API.result_first_character = Module.cwrap('OCRAD_result_first_character', 'number', ['number']);
	API._simple                = _simple;
	// END API SECTION //

	return API;
}

function createWebWorkerFromString(code){
	// http://stackoverflow.com/questions/5408406/web-workers-without-a-separate-javascript-file
	var blob;
	try {
	    blob = new Blob([code], {type: 'application/javascript'});
	} catch (e) { // Backwards-compatibility
	    var bb = window.BlobBuilder || window.WebKitBlobBuilder || window.MozBlobBuilder;
	    blob = new bb();
	    blob.append(code);
	    blob = blob.getBlob();
	}
	var worker;
	var URL = window.URL || window.webkitURL;
	try {
		worker = new Worker(URL.createObjectURL(blob));
	} catch (e) {};
	return worker;
}

function parseOcradResultsFile(raw){
	var tb;
	while(!(tb = raw.shift().match(/^total text blocks (\d+)/)));
	var blocks = [], all_lines = [], all_letters = [];
	for(var i = 0; i < parseInt(tb[1]); i++){
		var lines = [];
		var th = raw.shift().match(/^text block (\d+) (\d+) (\d+) (\d+) (\d+)/); // i x y w h
		var lc = raw.shift().match(/^lines (\d+)/);
		for(var j = 0; j < parseInt(lc[1]); j++){
			var letters = [];
			var cc = raw.shift().match(/^line (\d+) chars (\d+) height (\d+)/);
			for(var k = 0; k < parseInt(cc[2]); k++){
				var e = raw.shift().match(/^\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*;\s*(\d+)(\,?.+)?$/);
				var matches = [];
				if(parseInt(e[5]) > 0){
					var etc = e[6].trim();
					while(etc[0] == ',' && etc[1] == ' '){
						etc = etc.slice(2)
						var m = etc.match(/^\'(.+?)\'(\d+)/)
						matches.push({ letter: m[1], confidence: parseInt(m[2]) })
						etc = etc.slice(m[0].length)
					}
				}
				letters.push({
					x: parseInt(e[1]),
					y: parseInt(e[2]),
					width: parseInt(e[3]),
					height: parseInt(e[4]),
					matches: matches
				})
			}
			all_letters = all_letters.concat(letters)
			lines.push({
				height: parseInt(cc[3]),
				letters: letters
			})
		}
		all_lines = all_lines.concat(lines)
		blocks.push({
			x: parseInt(th[2]),
			y: parseInt(th[3]),
			width: parseInt(th[4]),
			height: parseInt(th[5]),
			lines: lines
		})
	}
	return {
		blocks: blocks,
		lines: all_lines,
		letters: all_letters
	};
}

var OCRAD = function OCRAD(image, arg1, arg2){
	// OCRAD(image) -> text
	// OCRAD(image, invert:boolean) -> text
	// OCRAD(image, invert:boolean, raw:function) -> text
	// OCRAD(image, options) -> text
	// OCRAD(image, options, callback) -> promise
	var opt = {}, async = false, rawfn;
	if(typeof arg1 == "object"){
		opt = arg1;
		if(typeof arg2 == "function") async = arg2;
	}else if(typeof arg1 == "function"){
		async = arg1;
	}else if(typeof arg1 == "boolean" && typeof arg2 == "function") {
		opt = { invert: arg1 }
		rawfn = arg2;
	}
	if(rawfn || opt.verbose) opt.raw = true;
	if(opt.numeric) opt.filters = ["numbers_only"]; 
	// for functions that may generate images
	if(typeof image == 'function') image = image();
	if(image.getContext){
		// for <canvas> elements
		image = image.getContext('2d');
	}else if(image.tagName == "IMG" || image.tagName == "VIDEO"){
		// for <video> or <image> elements
		var c = document.createElement('canvas');
		if(image.tagName == "IMG"){
			c.width  = image.naturalWidth;
			c.height = image.naturalHeight;
		}else if(image.tagName == "VIDEO"){
			c.width  = image.videoWidth;
			c.height = image.videoHeight;
		}
		var ctx = c.getContext('2d');
		ctx.drawImage(image, 0, 0);
		image = ctx;
	}
	// for canvas contexts
	if(image.getImageData) image = image.getImageData(0, 0, image.canvas.width, image.canvas.height);
	
	function postprocess(data){
		if(rawfn) data.split('\n').forEach(rawfn);
		if(opt.verbose) return parseOcradResultsFile(data.split('\n'));
		return data; // plain text probably
	}

	var worker;
	if(async){
		var code = 'var API = (' + createOcradInstance.toString() + ')(); onmessage = function(e){ postMessage(API._simple(e.data.image, e.data.opt)) }';
		worker = createWebWorkerFromString(code);
		if(worker){
			worker.onmessage = function(e){
				async(postprocess(e.data));
				worker.terminate();
				// TODO: perhaps reuse workers?
			}
			worker.postMessage({ image: image, opt: opt })
		}else{
			async(postprocess(OCRAD._simple(image, opt))); // pseudo-async
		}
	}else{
		return postprocess(OCRAD._simple(image, opt));
	}
}

var APISingleton;

function fwrap(name){
	return function(){
		OCRAD.preinit();
		return APISingleton[name].apply(APISingleton, arguments);
	}
}

OCRAD.preinit = function(){
	if(!APISingleton) {
		APISingleton = createOcradInstance();
	}
}

// BEGIN AUTOGENERATED //
OCRAD.set_print                      = fwrap('set_print');
OCRAD.write_file                     = fwrap('write_file');
OCRAD.read_file                      = fwrap('read_file');
OCRAD.read_text                      = fwrap('read_text');
OCRAD.delete_file                    = fwrap('delete_file');
OCRAD.version                        = fwrap('version');
OCRAD.open                           = fwrap('open');
OCRAD.close                          = fwrap('close');
OCRAD.get_errno                      = fwrap('get_errno');
OCRAD.set_image                      = fwrap('set_image');
OCRAD.set_image_from_file            = fwrap('set_image_from_file');
OCRAD.set_exportfile                 = fwrap('set_exportfile');
OCRAD.add_filter                     = fwrap('add_filter');
OCRAD.set_utf8_format                = fwrap('set_utf8_format');
OCRAD.set_threshold                  = fwrap('set_threshold');
OCRAD.scale                          = fwrap('scale');
OCRAD.transform                      = fwrap('transform');
OCRAD.recognize                      = fwrap('recognize');
OCRAD.result_blocks                  = fwrap('result_blocks');
OCRAD.result_lines                   = fwrap('result_lines');
OCRAD.result_chars_total             = fwrap('result_chars_total');
OCRAD.result_chars_block             = fwrap('result_chars_block');
OCRAD.result_chars_line              = fwrap('result_chars_line');
OCRAD.result_line                    = fwrap('result_line');
OCRAD.result_first_character         = fwrap('result_first_character');
OCRAD._simple                        = fwrap('_simple');
// END AUTOGENERATED //

return OCRAD;
})();

if (typeof module !== 'undefined' && module.exports) {
	module.exports = OCRAD;
}