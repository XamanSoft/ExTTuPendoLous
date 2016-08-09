// index.js
const addon = require('./build/Release/addon');

var template = module.exports;

template.__express =  template.render = addon.render;

template.renderSync = addon.renderSync;