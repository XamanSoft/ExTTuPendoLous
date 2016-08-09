{
	'target_defaults': {
		"include_dirs" : [ 
			"<!(node -e \"require('nan')\")",
			'include',
			'../include'
		]
	},
	
	"targets": [
	{
		"target_name": "addon",
		"dependencies": [
			"../build.gyp:extpl-lib"
		],
		"sources": [ "module.cc" ]
	}
  ]
}