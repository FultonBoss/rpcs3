﻿#pragma once
#include <sstream>

#include "GLSLTypes.h"
#include "ShaderParam.h"

#include "Utilities/StrFmt.h"

namespace program_common
{
	static void insert_compare_op(std::ostream& OS, bool low_precision)
	{
		if (low_precision)
		{
			OS <<
				"int compare(float a, float b)\n"
				"{\n"
				"	if (abs(a - b) < 0.000001) return 2;\n"
				"	return (a > b)? 4 : 1;\n"
				"}\n\n"

				"bool comparison_passes(float a, float b, uint func)\n"
				"{\n"
				"	if (func == 0) return false; // never\n"
				"	if (func == 7) return true;  // always\n\n"

				"	int op = compare(a, b);\n"
				"	switch (func)\n"
				"	{\n"
				"		case 1: return op == 1; // less\n"
				"		case 2: return op == 2; // equal\n"
				"		case 3: return op <= 2; // lequal\n"
				"		case 4: return op == 4; // greater\n"
				"		case 5: return op != 2; // nequal\n"
				"		case 6: return (op == 4 || op == 2); // gequal\n"
				"	}\n\n"

				"	return false; // unreachable\n"
				"}\n\n";
		}
		else
		{
			OS <<
			"bool comparison_passes(float a, float b, uint func)\n"
			"{\n"
			"	switch (func)\n"
			"	{\n"
			"		default:\n"
			"		case 0: return false; //never\n"
			"		case 1: return (a < b); //less\n"
			"		case 2: return (a == b); //equal\n"
			"		case 3: return (a <= b); //lequal\n"
			"		case 4: return (a > b); //greater\n"
			"		case 5: return (a != b); //nequal\n"
			"		case 6: return (a >= b); //gequal\n"
			"		case 7: return true; //always\n"
			"	}\n"
			"}\n\n";
		}
	}

	static void insert_compare_op_vector(std::ostream& OS)
	{
		OS <<
		"bvec4 comparison_passes(vec4 a, vec4 b, uint func)\n"
		"{\n"
		"	switch (func)\n"
		"	{\n"
		"		default:\n"
		"		case 0: return bvec4(false); //never\n"
		"		case 1: return lessThan(a, b); //less\n"
		"		case 2: return equal(a, b); //equal\n"
		"		case 3: return lessThanEqual(a, b); //lequal\n"
		"		case 4: return greaterThan(a, b); //greater\n"
		"		case 5: return notEqual(a, b); //nequal\n"
		"		case 6: return greaterThanEqual(a, b); //gequal\n"
		"		case 7: return bvec4(true); //always\n"
		"	}\n"
		"}\n\n";
	}

	static void insert_fog_declaration(std::ostream& OS, const std::string& wide_vector_type, const std::string& input_coord, bool declare = false)
	{
		std::string template_body;

		if (!declare)
			template_body += "$T fetch_fog_value(uint mode)\n";
		else
			template_body += "$T fetch_fog_value(uint mode, $T $I)\n";

		template_body +=
		"{\n"
		"	$T result = $T($I.x, 0., 0., 0.);\n"
		"	switch(mode)\n"
		"	{\n"
		"	default:\n"
		"		return result;\n"
		"	case 0:\n"
		"		//linear\n"
		"		result.y = fog_param1 * $I.x + (fog_param0 - 1.);\n"
		"		break;\n"
		"	case 1:\n"
		"		//exponential\n"
		"		result.y = exp(11.084 * (fog_param1 * $I.x + fog_param0 - 1.5));\n"
		"		break;\n"
		"	case 2:\n"
		"		//exponential2\n"
		"		result.y = exp(-pow(4.709 * (fog_param1 * $I.x + fog_param0 - 1.5), 2.));\n"
		"		break;\n"
		"	case 3:\n"
		"		//exponential_abs\n"
		"		result.y = exp(11.084 * (fog_param1 * abs($I.x) + fog_param0 - 1.5));\n"
		"		break;\n"
		"	case 4:\n"
		"		//exponential2_abs\n"
		"		result.y = exp(-pow(4.709 * (fog_param1 * abs($I.x) + fog_param0 - 1.5), 2.));\n"
		"		break;\n"
		" case 5:\n"
		"		//linear_abs\n"
		"		result.y = fog_param1 * abs($I.x) + (fog_param0 - 1.);\n"
		"		break;\n"
		"	}\n"
		"\n"
		"	result.y = clamp(result.y, 0., 1.);\n"
		"	return result;\n"
		"}\n\n";

		std::pair<std::string, std::string> replacements[] =
			{std::make_pair("$T", wide_vector_type),
			 std::make_pair("$I", input_coord)};

		OS << fmt::replace_all(template_body, replacements);
	}
}

namespace glsl
{
	static std::string getFloatTypeNameImpl(size_t elementCount)
	{
		switch (elementCount)
		{
		default:
			abort();
		case 1:
			return "float";
		case 2:
			return "vec2";
		case 3:
			return "vec3";
		case 4:
			return "vec4";
		}
	}

	static std::string getHalfTypeNameImpl(size_t elementCount)
	{
		switch (elementCount)
		{
		default:
			abort();
		case 1:
			return "float16_t";
		case 2:
			return "f16vec2";
		case 3:
			return "f16vec3";
		case 4:
			return "f16vec4";
		}
	}

	static std::string compareFunctionImpl(COMPARE f, const std::string &Op0, const std::string &Op1, bool scalar = false)
	{
		if (scalar)
		{
			switch (f)
			{
			case COMPARE::FUNCTION_SEQ:
				return Op0 + " == " + Op1;
			case COMPARE::FUNCTION_SGE:
				return Op0 + " >= " + Op1;
			case COMPARE::FUNCTION_SGT:
				return Op0 + " > " + Op1;
			case COMPARE::FUNCTION_SLE:
				return Op0 + " <= " + Op1;
			case COMPARE::FUNCTION_SLT:
				return Op0 + " < " + Op1;
			case COMPARE::FUNCTION_SNE:
				return Op0 + " != " + Op1;
			}
		}
		else
		{
			switch (f)
			{
			case COMPARE::FUNCTION_SEQ:
				return "equal(" + Op0 + ", " + Op1 + ")";
			case COMPARE::FUNCTION_SGE:
				return "greaterThanEqual(" + Op0 + ", " + Op1 + ")";
			case COMPARE::FUNCTION_SGT:
				return "greaterThan(" + Op0 + ", " + Op1 + ")";
			case COMPARE::FUNCTION_SLE:
				return "lessThanEqual(" + Op0 + ", " + Op1 + ")";
			case COMPARE::FUNCTION_SLT:
				return "lessThan(" + Op0 + ", " + Op1 + ")";
			case COMPARE::FUNCTION_SNE:
				return "notEqual(" + Op0 + ", " + Op1 + ")";
			}
		}

		fmt::throw_exception("Unknown compare function" HERE);
	}

	static void insert_vertex_input_fetch(std::stringstream& OS, glsl_rules rules, bool glsl4_compliant=true)
	{
		std::string vertex_id_name = (rules == glsl_rules_opengl4) ? "gl_VertexID" : "gl_VertexIndex";

		//Actually decode a vertex attribute from a raw byte stream
		OS <<
		"struct attribute_desc\n"
		"{\n"
		"	uint type;\n"
		"	uint attribute_size;\n"
		"	uint starting_offset;\n"
		"	uint stride;\n"
		"	uint frequency;\n"
		"	bool swap_bytes;\n"
		"	bool is_volatile;\n"
		"	bool modulo;\n"
		"};\n\n"

		"uint get_bits(uint x, uint y, uint z, uint w, bool swap)\n"
		"{\n"
		"	if (swap) return (w | z << 8 | y << 16 | x << 24);\n"
		"	return (x | y << 8 | z << 16 | w << 24);\n"
		"}\n\n"

		"uint get_bits(uint x, uint y, bool swap)\n"
		"{\n"
		"	if (swap) return (y | x << 8);\n"
		"	return (x | y << 8);\n"
		"}\n\n"

		"int preserve_sign_s16(uint bits)\n"
		"{\n"
		"	//convert raw 16 bit value into signed 32-bit integer counterpart\n"
		"	uint sign = bits & 0x8000;\n"
		"	if (sign != 0) bits |= 0xFFFF0000;\n"
		"	return int(bits);\n"
		"}\n\n"

		"#define get_s16(v, s) preserve_sign_s16(get_bits(v, s))\n\n";

		//For intel GPUs which cannot access vectors in indexed mode (driver bug? or glsl version too low?)
		if (!glsl4_compliant)
		{
			OS <<
			"void mov(inout vec4 vector, in int index, in float scalar)\n"
			"{\n"
			"	switch(index)\n"
			"	{\n"
			"		case 0: vector.x = scalar; return;\n"
			"		case 1: vector.y = scalar; return;\n"
			"		case 2: vector.z = scalar; return;\n"
			"		case 3: vector.w = scalar; return;\n"
			"	}\n"
			"}\n";

			OS <<
			"uint ref(in uvec4 vector, in int index)\n"
			"{\n"
			"	switch(index)\n"
			"	{\n"
			"		case 0: return vector.x;\n"
			"		case 1: return vector.y;\n"
			"		case 2: return vector.z;\n"
			"		case 3: return vector.w;\n"
			"	}\n"
			"}\n";
		}
		else
		{
			OS <<
			"#define mov(v, i, s) v[i] = s\n"
			"#define ref(v, i) v[i]\n\n";
		}

		OS <<
		"vec4 fetch_attribute(attribute_desc desc, int vertex_id, usamplerBuffer input_stream)\n"
		"{\n"
		"	vec4 result = vec4(0., 0., 0., 1.);\n"
		"	vec4 scale = vec4(1.);\n"
		"	bool reverse_order = false;\n"
		"\n"
		"	const int elem_size_table[] = { 2, 4, 2, 1, 2, 4, 1 };\n"
		"	const int elem_size = elem_size_table[desc.type];\n"
		"	uvec4 tmp;\n"
		"\n"
		"	int n;\n"
		"	int i = int((vertex_id * desc.stride) + desc.starting_offset);\n"
		"\n"
		"	for (n = 0; n < desc.attribute_size; n++)\n"
		"	{\n"
		"		tmp.x = texelFetch(input_stream, i++).x;\n"
		"		if (elem_size == 2)\n"
		"		{\n"
		"			tmp.y = texelFetch(input_stream, i++).x;\n"
		"			tmp.x = get_bits(tmp.x, tmp.y, desc.swap_bytes);\n"
		"		}\n"
		"		else if (elem_size == 4)\n"
		"		{\n"
		"			tmp.y = texelFetch(input_stream, i++).x;\n"
		"			tmp.z = texelFetch(input_stream, i++).x;\n"
		"			tmp.w = texelFetch(input_stream, i++).x;\n"
		"			tmp.x = get_bits(tmp.x, tmp.y, tmp.z, tmp.w, desc.swap_bytes);\n"
		"		}\n"
		"\n"
		"		switch (desc.type)\n"
		"		{\n"
		"		case 0:\n"
		"			//signed normalized 16-bit\n"
		"			mov(scale, n, 32767.);\n"
		"		case 4:\n"
		"			//signed word\n"
		"			mov(result, n, preserve_sign_s16(tmp.x));\n"
		"			break;\n"
		"		case 1:\n"
		"			//float\n"
		"			mov(result, n, uintBitsToFloat(tmp.x));\n"
		"			break;\n"
		"		case 2:\n"
		"			//half\n"
		"			mov(result, n, unpackHalf2x16(tmp.x).x);\n"
		"			break;\n"
		"		case 3:\n"
		"			//unsigned byte\n"
		"			mov(scale, n, 255.);\n"
		"		case 6:\n"
		"			//ub256\n"
		"			mov(result, n, tmp.x);\n"
		"			reverse_order = desc.swap_bytes;\n"
		"			break;\n"
		"		case 5:\n"
		"			//cmp\n"
		"			result.x = preserve_sign_s16((tmp.x & 0x7FF) << 5);\n"
		"			result.y = preserve_sign_s16(((tmp.x >> 11) & 0x7FF) << 5);\n"
		"			result.z = preserve_sign_s16(((tmp.x >> 22) & 0x3FF) << 6);\n"
		"			result.w = 1.;\n"
		"			scale = vec4(32767., 32767., 32767., 1.);\n"
		"			break;\n"
		"		}\n"
		"	}\n"
		"\n"
		"	result /= scale;\n"
		"	return (reverse_order)? result.wzyx: result;\n"
		"}\n\n"

		"attribute_desc fetch_desc(int location)\n"
		"{\n"
		"	// Each descriptor is 64 bits wide\n"
		"	// [0-8] attribute stride\n"
		"	// [8-24] attribute divisor\n"
		"	// [24-27] attribute type\n"
		"	// [27-30] attribute size\n"
		"	// [30-31] reserved\n"
		"	// [32-60] starting offset\n"
		"	// [60-61] swap bytes flag\n"
		"	// [61-62] volatile flag\n"
		"	// [62-63] modulo enable flag\n";

		if (rules == glsl_rules_opengl4)
		{
			// Data is packed into a ubo
			OS <<
			"	int block = (location >> 1);\n"
			"	int sub_block = (location & 1) << 1;\n"
			"	uvec2 attrib = uvec2(\n"
			"		ref(input_attributes_blob[block], sub_block + 0),\n"
			"		ref(input_attributes_blob[block], sub_block + 1));\n";
		}
		else
		{
			// Fetch parameters streamed separately from draw parameters
			OS <<
			"	location += int(layout_ptr_offset);\n"
			"	uvec2 attrib = texelFetch(vertex_layout_stream, location).xy;\n\n";
		}

		OS <<
		"	attribute_desc result;\n"
		"	result.stride = attrib.x & 0xFF;\n"
		"	result.frequency = (attrib.x >> 8) & 0xFFFF;\n"
		"	result.type = (attrib.x >> 24) & 0x7;\n"
		"	result.attribute_size = (attrib.x >> 27) & 0x7;\n"
		"	result.starting_offset = (attrib.y & 0x1FFFFFFF);\n"
		"	result.swap_bytes = ((attrib.y >> 29) & 0x1) != 0;\n"
		"	result.is_volatile = ((attrib.y >> 30) & 0x1) != 0;\n"
		"	result.modulo = ((attrib.y >> 31) & 0x1) != 0;\n"
		"	return result;\n"
		"}\n\n"

		"vec4 read_location(int location)\n"
		"{\n"
		"	attribute_desc desc = fetch_desc(location);\n"
		"	if (desc.attribute_size == 0)\n"
		"	{\n"
		"		//default values\n"
		"		const vec4 defaults[] = \n"
		"		{	vec4(0., 0., 0., 1.), //position\n"
		"			vec4(0.), vec4(0.), //weight, normals\n"
		"			vec4(1.), //diffuse\n"
		"			vec4(0.), vec4(0.), //specular, fog\n"
		"			vec4(1.), //point size\n"
		"			vec4(0.), //in_7\n"
		"			//in_tc registers\n"
		"			vec4(0.), vec4(0.), vec4(0.), vec4(0.),\n"
		"			vec4(0.), vec4(0.), vec4(0.), vec4(0.)\n"
		"		};\n"
		"		return defaults[location];\n"
		"	}\n\n"
		"	int vertex_id = " << vertex_id_name << " - int(vertex_base_index);\n"
		"	if (desc.frequency == 0)\n"
		"	{\n"
		"		vertex_id = 0;\n"
		"	}\n"
		"	else if (desc.modulo)\n"
		"	{\n"
		"		//if a vertex modifier is active; vertex_base must be 0 and is ignored\n"
		"		vertex_id = (" << vertex_id_name << " + int(vertex_index_offset)) % int(desc.frequency);\n"
		"	}\n"
		"	else\n"
		"	{\n"
		"		vertex_id /= int(desc.frequency); \n"
		"	}\n"
		"\n"
		"	if (desc.is_volatile)\n"
		"		return fetch_attribute(desc, vertex_id, volatile_input_stream);\n"
		"	else\n"
		"		return fetch_attribute(desc, vertex_id, persistent_input_stream);\n"
		"}\n\n";
	}

	static void insert_rop(std::ostream& OS, bool _32_bit_exports, bool native_half_support, bool emulate_coverage_tests)
	{
		const std::string reg0 = _32_bit_exports ? "r0" : "h0";
		const std::string reg1 = _32_bit_exports ? "r2" : "h4";
		const std::string reg2 = _32_bit_exports ? "r3" : "h6";
		const std::string reg3 = _32_bit_exports ? "r4" : "h8";

		//TODO: Implement all ROP options like CSAA and ALPHA_TO_ONE here
		OS <<
		"	if ((rop_control & 0xFF) != 0)\n"
		"	{\n"
		"		bool alpha_test = (rop_control & 0x1) > 0;\n"
		"		uint alpha_func = ((rop_control >> 16) & 0x7);\n";

		if (!_32_bit_exports)
		{
			OS << "		bool srgb_convert = (rop_control & 0x2) > 0;\n\n";
		}

		if (emulate_coverage_tests)
		{
			OS << "		bool a2c_enabled = (rop_control & 0x10) > 0;\n";
		}

		OS <<
		"		if (alpha_test && !comparison_passes(" << reg0 << ".a, alpha_ref, alpha_func))\n"
		"		{\n"
		"			discard;\n"
		"		}\n";

		if (emulate_coverage_tests)
		{
			OS <<
			"		else if (a2c_enabled && !coverage_test_passes(" << reg0 << ", rop_control >> 5))\n"
			"		{\n"
			"			discard;\n"
			"		}\n";
		}

		if (!_32_bit_exports)
		{
			// Tested using NPUB90375; some shaders (32-bit output only?) do not obey srgb flags
			if (native_half_support)
			{
				OS <<
				"		else if (srgb_convert)\n"
				"		{\n"
				"			" << reg0 << ".rgb = clamp16(linear_to_srgb(" << reg0 << ")).rgb;\n"
				"			" << reg1 << ".rgb = clamp16(linear_to_srgb(" << reg1 << ")).rgb;\n"
				"			" << reg2 << ".rgb = clamp16(linear_to_srgb(" << reg2 << ")).rgb;\n"
				"			" << reg3 << ".rgb = clamp16(linear_to_srgb(" << reg3 << ")).rgb;\n"
				"		}\n";
			}
			else
			{
				OS <<
				"		else if (srgb_convert)\n"
				"		{\n"
				"			" << reg0 << ".rgb = linear_to_srgb(" << reg0 << ").rgb;\n"
				"			" << reg1 << ".rgb = linear_to_srgb(" << reg1 << ").rgb;\n"
				"			" << reg2 << ".rgb = linear_to_srgb(" << reg2 << ").rgb;\n"
				"			" << reg3 << ".rgb = linear_to_srgb(" << reg3 << ").rgb;\n"
				"		}\n";
			}
		}

		OS <<
		"	}\n\n"

		"	ocol0 = " << reg0 << ";\n"
		"	ocol1 = " << reg1 << ";\n"
		"	ocol2 = " << reg2 << ";\n"
		"	ocol3 = " << reg3 << ";\n\n";
	}

	static void insert_glsl_legacy_function(std::ostream& OS, const shader_properties& props)
	{
		OS << "#define _select mix\n";
		OS << "#define _saturate(x) clamp(x, 0., 1.)\n";
		OS << "#define _rand(seed) fract(sin(dot(seed.xy, vec2(12.9898f, 78.233f))) * 43758.5453f)\n\n";

		if (props.require_lit_emulation)
		{
			OS <<
			"vec4 lit_legacy(vec4 val)"
			"{\n"
			"	vec4 clamped_val = val;\n"
			"	clamped_val.x = max(val.x, 0.);\n"
			"	clamped_val.y = max(val.y, 0.);\n"
			"	vec4 result;\n"
			"	result.x = 1.;\n"
			"	result.w = 1.;\n"
			"	result.y = clamped_val.x;\n"
			"	result.z = clamped_val.x > 0. ? exp(clamped_val.w * log(max(clamped_val.y, 0.0000000001))) : 0.;\n"
			"	return result;\n"
			"}\n\n";
		}

		if (props.domain == glsl::program_domain::glsl_vertex_program)
		{
			OS <<
			"vec4 apply_zclip_xform(vec4 pos, float near_plane, float far_plane)\n"
			"{\n"
			"	float d = pos.z / pos.w;\n"
			"	if (d < 0.f && d >= near_plane)\n"
			"		d = 0.f;\n" //force clamp negative values
			"	else if (d > 1.f && d <= far_plane)\n"
			"		d = min(1., 0.99 + (0.01 * (pos.z - near_plane) / (far_plane - near_plane)));\n"
			"	else\n"
			"		return pos; //d = (0.99 * d);\n" //range compression for normal values is disabled until a solution to ops comparing z is found
			"\n"
			"	pos.z = d * pos.w;\n"
			"	return pos;\n"
			"}\n\n";

			return;
		}

		program_common::insert_compare_op(OS, props.low_precision_tests);

		if (props.require_shadow_ops && props.emulate_shadow_compare)
		{
			program_common::insert_compare_op_vector(OS);
		}

		if (props.emulate_coverage_tests)
		{
			// NOTES:
			// Lowers alpha accuracy down to 2 bits, to mimic A2C banding
			// Alpha lower than the real threshold (e.g 0.25 for 4 samples) gets a randomized chance to make it to the lowest transparency state
			// Helps to avoid A2C tested foliage disappearing in the distance
			OS <<
			"bool coverage_test_passes(/*inout*/in vec4 _sample, uint control)\n"
			"{\n"
			"	if ((control & 0x1) == 0) return false;\n"
			"\n"
			"	float samples = ((control & 0x2) != 0)? 4.f : 2.f;\n"
			"	float hash    = _saturate(_rand(gl_FragCoord) + 0.5f) * 0.9f;\n"
			"	float epsilon = hash / samples;\n"
			"	float alpha   = trunc((_sample.a + epsilon) * samples) / samples;\n"
			"	//_sample.a     = min(_sample.a, alpha);\n" // Cannot blend A2C samples naively as they are order independent! Causes background bleeding
			"	return (alpha > 0.f);\n"
			"}\n\n";
		}

		if (!props.fp32_outputs)
		{
			OS <<
			"vec4 linear_to_srgb(vec4 cl)\n"
			"{\n"
			"	vec4 low = cl * 12.92;\n"
			"	vec4 high = 1.055 * pow(cl, vec4(1. / 2.4)) - 0.055;\n"
			"	bvec4 select = lessThan(cl, vec4(0.0031308));\n"
			"	return clamp(mix(high, low, select), 0., 1.);\n"
			"}\n\n";
		}

		if (props.require_depth_conversion)
		{
			//NOTE: Memory layout is fetched as byteswapped BGRA [GBAR] (GOW collection, DS2, DeS)
			//The A component (Z) is useless (should contain stencil8 or just 1)
			OS <<
			"vec4 decodeLinearDepth(float depth_value)\n"
			"{\n"
			"	uint value = uint(depth_value * 16777215.);\n"
			"	uint b = (value & 0xff);\n"
			"	uint g = (value >> 8) & 0xff;\n"
			"	uint r = (value >> 16) & 0xff;\n"
			"	return vec4(float(g)/255., float(b)/255., 1., float(r)/255.);\n"
			"}\n\n"

			"float read_value(vec4 src, uint remap_index)\n"
			"{\n"
			"	switch (remap_index)\n"
			"	{\n"
			"		case 0: return src.a;\n"
			"		case 1: return src.r;\n"
			"		case 2: return src.g;\n"
			"		case 3: return src.b;\n"
			"	}\n"
			"}\n\n"

			"vec4 texture2DReconstruct(sampler2D tex, usampler2D stencil_tex, vec2 coord, float remap)\n"
			"{\n"
			"	vec4 result = decodeLinearDepth(texture(tex, coord.xy).r);\n"
			"	result.z = float(texture(stencil_tex, coord.xy).x) / 255.f;\n"
			"	uint remap_vector = floatBitsToUint(remap) & 0xFF;\n"
			"	if (remap_vector == 0xE4) return result;\n\n"
			"	vec4 tmp;\n"
			"	uint remap_a = remap_vector & 0x3;\n"
			"	uint remap_r = (remap_vector >> 2) & 0x3;\n"
			"	uint remap_g = (remap_vector >> 4) & 0x3;\n"
			"	uint remap_b = (remap_vector >> 6) & 0x3;\n"
			"	tmp.a = read_value(result, remap_a);\n"
			"	tmp.r = read_value(result, remap_r);\n"
			"	tmp.g = read_value(result, remap_g);\n"
			"	tmp.b = read_value(result, remap_b);\n"
			"	return tmp;\n"
			"}\n\n";
		}

		if (props.require_texture_ops)
		{
			if (props.require_shadow_ops && props.emulate_shadow_compare)
			{
				OS <<
				"vec4 shadowCompare(sampler2D tex, vec3 p, uint func)\n"
				"{\n"
				"	vec4 samples = textureGather(tex, p.xy).xxxx;\n"
				"	vec4 ref = clamp(p.z, 0., 1.).xxxx;\n"
				"	vec4 filtered = vec4(comparison_passes(samples, ref, func));\n"
				"	return filtered * dot(filtered, vec4(0.25f));\n"
				"}\n\n"

				"vec4 shadowCompareProj(sampler2D tex, vec4 p, uint func)\n"
				"{\n"
				"	return shadowCompare(tex, p.xyz / p.w, func);\n"
				"}\n\n";
			}

			OS <<

#ifdef __APPLE__
			"vec4 remap_vector(vec4 rgba, uint remap_bits)\n"
			"{\n"
			"	uvec4 selector = (uvec4(remap_bits) >> uvec4(3, 6, 9, 0)) & 0x7;\n"
			"	bvec4 choice = greaterThan(selector, uvec4(1));\n"
			"\n"
			"	vec4 direct = vec4(selector);\n"
			"	selector = min(selector - 2, selector);\n"
			"	vec4 indexed = vec4(rgba[selector.r], rgba[selector.g], rgba[selector.b], rgba[selector.a]);\n"
			"	return mix(direct, indexed, choice);\n"
			"}\n\n"
#endif
			"vec4 srgb_to_linear(vec4 cs)\n"
			"{\n"
			"	vec4 a = cs / 12.92;\n"
			"	vec4 b = pow((cs + 0.055) / 1.055, vec4(2.4));\n"
			"	return _select(a, b, greaterThan(cs, vec4(0.04045)));\n"
			"}\n\n"

			//TODO: Move all the texture read control operations here
			"vec4 process_texel(vec4 rgba, uint control_bits)\n"
			"{\n"
#ifdef __APPLE__
			"	uint remap_bits = (control_bits >> 16) & 0xFFFF;\n"
			"	if (remap_bits != 0x8D5) rgba = remap_vector(rgba, remap_bits);\n\n"
#endif
			"	if (control_bits == 0)\n"
			"	{\n"
			"		return rgba;\n"
			"	}\n"
			"\n"
			"	if ((control_bits & 0x10) != 0)\n"
			"	{\n"
			"		// Alphakill\n"
			"		if (rgba.a < 0.000001)\n"
			"		{\n"
			"			discard;\n"
			"			return rgba;\n"
			"		}\n"
			"	}\n"
			"\n"
			"	//TODO: Verify gamma control bit ordering, looks to be 0x7 for rgb, 0xF for rgba\n"
			"	uvec4 mask = uvec4(control_bits & 0xF) & uvec4(0x1, 0x2, 0x4, 0x8);\n"
			"	vec4 convert = srgb_to_linear(rgba);\n"
			"	return _select(rgba, convert, notEqual(mask, uvec4(0)));\n"
			"}\n\n"

			"#define TEX_NAME(index) tex##index\n"
			"#define TEX_NAME_STENCIL(index) tex##index##_stencil\n\n"

			"#define TEX1D(index, coord1) process_texel(texture(TEX_NAME(index), coord1 * texture_parameters[index].x), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX1D_BIAS(index, coord1, bias) process_texel(texture(TEX_NAME(index), coord1 * texture_parameters[index].x, bias), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX1D_LOD(index, coord1, lod) process_texel(textureLod(TEX_NAME(index), coord1 * texture_parameters[index].x, lod), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX1D_GRAD(index, coord1, dpdx, dpdy) process_texel(textureGrad(TEX_NAME(index), coord1 * texture_parameters[index].x, dpdx, dpdy), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX1D_PROJ(index, coord2) process_texel(textureProj(TEX_NAME(index), coord2 * vec2(texture_parameters[index].x, 1.)), floatBitsToUint(texture_parameters[index].w))\n"

			"#define TEX2D(index, coord2) process_texel(texture(TEX_NAME(index), coord2 * texture_parameters[index].xy), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX2D_BIAS(index, coord2, bias) process_texel(texture(TEX_NAME(index), coord2 * texture_parameters[index].xy, bias), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX2D_LOD(index, coord2, lod) process_texel(textureLod(TEX_NAME(index), coord2 * texture_parameters[index].xy, lod), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX2D_GRAD(index, coord2, dpdx, dpdy) process_texel(textureGrad(TEX_NAME(index), coord2 * texture_parameters[index].xy, dpdx, dpdy), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX2D_PROJ(index, coord4) process_texel(textureProj(TEX_NAME(index), coord4 * vec4(texture_parameters[index].xy, 1., 1.)), floatBitsToUint(texture_parameters[index].w))\n"

			"#define TEX2D_DEPTH_RGBA8(index, coord2) process_texel(texture2DReconstruct(TEX_NAME(index), TEX_NAME_STENCIL(index), coord2 * texture_parameters[index].xy, texture_parameters[index].z), floatBitsToUint(texture_parameters[index].w))\n";

			if (props.emulate_shadow_compare)
			{
				OS <<
				"#define TEX2D_SHADOW(index, coord3) shadowCompare(TEX_NAME(index), coord3 * vec3(texture_parameters[index].xy, 1.), floatBitsToUint(texture_parameters[index].w) >> 8)\n"
				"#define TEX2D_SHADOWPROJ(index, coord4) shadowCompareProj(TEX_NAME(index), coord4 * vec4(texture_parameters[index].xy, 1., 1.), floatBitsToUint(texture_parameters[index].w) >> 8)\n";
			}
			else
			{
				OS <<
				"#define TEX2D_SHADOW(index, coord3) texture(TEX_NAME(index), coord3 * vec3(texture_parameters[index].xy, 1.))\n"
				"#define TEX2D_SHADOWPROJ(index, coord4) textureProj(TEX_NAME(index), coord4 * vec4(texture_parameters[index].xy, 1., 1.))\n";
			}

			OS <<
			"#define TEX3D(index, coord3) process_texel(texture(TEX_NAME(index), coord3), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX3D_BIAS(index, coord3, bias) process_texel(texture(TEX_NAME(index), coord3, bias), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX3D_LOD(index, coord3, lod) process_texel(textureLod(TEX_NAME(index), coord3, lod), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX3D_GRAD(index, coord3, dpdx, dpdy) process_texel(textureGrad(TEX_NAME(index), coord3, dpdx, dpdy), floatBitsToUint(texture_parameters[index].w))\n"
			"#define TEX3D_PROJ(index, coord4) process_texel(textureProj(TEX_NAME(index), coord4), floatBitsToUint(texture_parameters[index].w))\n\n";
		}

		if (props.require_wpos)
		{
			OS <<
			"vec4 get_wpos()\n"
			"{\n"
			"	float abs_scale = abs(wpos_scale);\n"
			"	return (gl_FragCoord * vec4(abs_scale, wpos_scale, 1., 1.)) + vec4(0., wpos_bias, 0., 0.);\n"
			"}\n\n";
		}
	}

	static void insert_fog_declaration(std::ostream& OS)
	{
		program_common::insert_fog_declaration(OS, "vec4", "fog_c");
	}

	static std::string getFunctionImpl(FUNCTION f)
	{
		switch (f)
		{
		default:
			abort();
		case FUNCTION::FUNCTION_DP2:
			return "$Ty(dot($0.xy, $1.xy))";
		case FUNCTION::FUNCTION_DP2A:
			return "$Ty(dot($0.xy, $1.xy) + $2.x)";
		case FUNCTION::FUNCTION_DP3:
			return "$Ty(dot($0.xyz, $1.xyz))";
		case FUNCTION::FUNCTION_DP4:
			return "$Ty(dot($0, $1))";
		case FUNCTION::FUNCTION_DPH:
			return "$Ty(dot(vec4($0.xyz, 1.0), $1))";
		case FUNCTION::FUNCTION_SFL:
			return "$Ty(0., 0., 0., 0.)";
		case FUNCTION::FUNCTION_STR:
			return "$Ty(1., 1., 1., 1.)";
		case FUNCTION::FUNCTION_FRACT:
			return "fract($0)";
		case FUNCTION::FUNCTION_REFL:
			return "$Ty($0 - 2.0 * (dot($0, $1)) * $1)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE1D:
			return "TEX1D($_i, $0.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE1D_BIAS:
			return "TEX1D_BIAS($_i, $0.x, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE1D_PROJ:
			return "TEX1D_PROJ($_i, $0.xy)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE1D_LOD:
			return "TEX1D_LOD($_i, $0.x, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE1D_GRAD:
			return "TEX1D_GRAD($_i, $0.x, $1.x, $2.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE2D:
			return "TEX2D($_i, $0.xy)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE2D_BIAS:
			return "TEX2D_BIAS($_i, $0.xy, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE2D_PROJ:
			return "TEX2D_PROJ($_i, $0)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE2D_LOD:
			return "TEX2D_LOD($_i, $0.xy, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE2D_GRAD:
			return "TEX2D_GRAD($_i, $0.xy, $1.xy, $2.xy)";
		case FUNCTION::FUNCTION_TEXTURE_SHADOW2D:
			return "TEX2D_SHADOW($_i, $0.xyz)";
		case FUNCTION::FUNCTION_TEXTURE_SHADOW2D_PROJ:
			return "TEX2D_SHADOWPROJ($_i, $0)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLECUBE:
			return "TEX3D($_i, $0.xyz)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLECUBE_BIAS:
			return "TEX3D_BIAS($_i, $0.xyz, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLECUBE_PROJ:
			return "TEX3D($_i, ($0.xyz / $0.w))";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLECUBE_LOD:
			return "TEX3D_LOD($_i, $0.xyz, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLECUBE_GRAD:
			return "TEX3D_GRAD($_i, $0.xyz, $1.xyz, $2.xyz)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE3D:
			return "TEX3D($_i, $0.xyz)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE3D_BIAS:
			return "TEX3D_BIAS($_i, $0.xyz, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE3D_PROJ:
			return "TEX3D_PROJ($_i, $0)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE3D_LOD:
			return "TEX3D_LOD($_i, $0.xyz, $1.x)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE3D_GRAD:
			return "TEX3D_GRAD($_i, $0.xyz, $1.xyz, $2.xyz)";
		case FUNCTION::FUNCTION_DFDX:
			return "dFdx($0)";
		case FUNCTION::FUNCTION_DFDY:
			return "dFdy($0)";
		case FUNCTION::FUNCTION_VERTEX_TEXTURE_FETCH1D:
			return "textureLod($t, $0.x, 0)";
		case FUNCTION::FUNCTION_VERTEX_TEXTURE_FETCH2D:
			return "textureLod($t, $0.xy, 0)";
		case FUNCTION::FUNCTION_VERTEX_TEXTURE_FETCH3D:
		case FUNCTION::FUNCTION_VERTEX_TEXTURE_FETCHCUBE:
			return "textureLod($t, $0.xyz, 0)";
		case FUNCTION::FUNCTION_TEXTURE_SAMPLE2D_DEPTH_RGBA:
			return "TEX2D_DEPTH_RGBA8($_i, $0.xy)";
		}
	}
}
