var api = { // low level api
	'version': Module.cwrap('OCRAD_version', 'string'),
	'open': Module.cwrap('OCRAD_open', 'number'),
	'close': Module.cwrap('OCRAD_close', 'number', ['number']),
	'get_errno': Module.cwrap('OCRAD_get_errno', 'number', ['number']),
	'set_image': Module.cwrap('OCRAD_set_image', 'number', ['number', 'number', 'number']),
	'set_image_from_file': Module.cwrap('OCRAD_set_image_from_file', 'number', ['number', 'string', 'number']),
	'set_utf8_format': Module.cwrap('OCRAD_set_utf8_format', 'number', ['number', 'number']),
	'set_threshold': Module.cwrap('OCRAD_set_threshold', 'number', ['number', 'number']),
	'scale': Module.cwrap('OCRAD_scale', 'number', ['number', 'number']),
	'recognize': Module.cwrap('OCRAD_recognize', 'number', ['number', 'number']),
	'result_blocks': Module.cwrap('OCRAD_result_blocks', 'number', ['number']),
	'result_lines': Module.cwrap('OCRAD_result_lines', 'number', ['number', 'number']),
	'result_chars_total': Module.cwrap('OCRAD_result_chars_total', 'number', ['number']),
	'result_chars_block': Module.cwrap('OCRAD_result_chars_block', 'number', ['number', 'number']),
	'result_chars_line': Module.cwrap('OCRAD_result_chars_line', 'number', ['number', 'number', 'number']),
	'result_line': Module.cwrap('OCRAD_result_line', 'string', ['number', 'number', 'number']),
	'result_first_character': Module.cwrap('OCRAD_result_first_character', 'number', ['number'])
}



return {
	api: api,
	version: function(){ return api.version() },
	ocr: function(){
		var desc = api.open()

		api.close(desc)
	}
}

})();