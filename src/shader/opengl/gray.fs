// Copyright (c) 2022 Xiaomi Guo
// Modern Ruby Game Engine (RGM) is licensed under Mulan PSL v2.
// You can use this software according to the terms and conditions of the Mulan PSL v2.
// You may obtain a copy of Mulan PSL v2 at:
//          http://license.coscl.org.cn/MulanPSL2
// THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
// EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
// MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
// See the Mulan PSL v2 for more details.

varying vec2 v_texCoord;
uniform sampler2D tex0;

void main()
{
	vec4 color = texture2D(tex0, v_texCoord);
	float gray = color.r * 0.299 + color.g * 0.587 + color.b * 0.114;
	color.rgb = vec3(gray, gray, gray);	
	// Return the final color
	gl_FragColor = color;
}