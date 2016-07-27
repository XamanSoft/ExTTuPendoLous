# ExTTuPendoLous (under construction)
Extended Tag Template - simple extended tag system for templating

## Description

A template system based into a macro language, where you can build your own macro blocks.

It works like C macro language, inserting text/data where a macro block appears, every macro start with
a at symbol (@).

## Macro blocks

Macro block start with a optional name and open/close symbol, blocks accept any kind of text. The format
of the text can be defined by developer.

## Macro blocks examples

```
@vars{
	title: 'Home',
	content: 'index',
}

@consts{
	true: 1,
	false: 0,
	pi: 3.14159
}

@js{
	// js code to run
	// for a code block the end symbol must not have a white space
	$.print("}"); // this passes
}

@js["include_file.js"]
@["include_file"]
@{"print this text"}
@{var_to_print}

@rm{
	Comment/Descarded block
}
```

## Default macros (coming soon)

Default/basic implemented macros (some are show above). 

## Example

```html
@rm{
	File: test.html.xtpl
	Description: This is a example of template file
}<!DOCTYPE html>
<html>
<head>
	<title>@{title}</title>
</head>
<body>
@[content]
</body>
</html>
```