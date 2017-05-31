#version 320 es
precision highp float;

out vec4 colorOut;

void main(){
	//colorOut = vec4(gl_FragCoord.x/1920.0, gl_FragCoord.y/1080.0, 0.0, 1.0);
	colorOut = vec4(1.0, 1.0, 1.0, 1.0);
}
