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

uniform vec4 k;

void main()
{
	vec4 color = texture2D(tex0, v_texCoord);
	vec3 c = color.rgb;
	color.r = c.r * k.x + c.g * k.y + c.b * k.z;
    color.g = c.g * k.x + c.b * k.y + c.r * k.z;
    color.b = c.b * k.x + c.r * k.y + c.g * k.z;
	// Return the final color
	gl_FragColor = color;
}