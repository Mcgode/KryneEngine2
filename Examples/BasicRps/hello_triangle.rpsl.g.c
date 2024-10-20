/* Provide Declarations */
#include <stdarg.h>
#include <setjmp.h>
#include <limits.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#ifndef __cplusplus
typedef unsigned char bool;
#endif

/* get a declaration for alloca */
#if defined(__CYGWIN__) || defined(__MINGW32__)
#define  alloca(x) __builtin_alloca((x))
#define _alloca(x) __builtin_alloca((x))
#elif defined(__APPLE__)
extern void *__builtin_alloca(unsigned long);
#define alloca(x) __builtin_alloca(x)
#define longjmp _longjmp
#define setjmp _setjmp
#elif defined(__sun__)
#if defined(__sparcv9)
extern void *__builtin_alloca(unsigned long);
#else
extern void *__builtin_alloca(unsigned int);
#endif
#define alloca(x) __builtin_alloca(x)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || defined(__arm__)
#define alloca(x) __builtin_alloca(x)
#elif defined(_MSC_VER)
#define alloca(x) _alloca(x)
#else
#include <alloca.h>
#endif

#ifndef _MSC_VER
#define __forceinline __attribute__((always_inline)) inline
#endif

#if defined(__GNUC__)
#define  __ATTRIBUTELIST__(x) __attribute__(x)
#else
#define  __ATTRIBUTELIST__(x)  
#endif

#ifdef _MSC_VER  /* Can only support "linkonce" vars with GCC */
#define __attribute__(X)
#endif

#ifdef _MSC_VER  /* Handle __declspec(dllexport) */
#define __CBE_DLLEXPORT__ __declspec(dllexport)
#define __CBE_DLLIMPORT__ __declspec(dllimport)
#else
#define __CBE_DLLEXPORT__ __attribute__ ((visibility ("default")))
#define __CBE_DLLIMPORT__ __attribute__ ((visibility ("default")))
#endif

#define __asm__(X) /* RPS: Ignore __asm__(Name) */

#if defined(_MSC_VER) && !defined(__clang__)
#define __MSALIGN__(X) __declspec(align(X))
#else
#define __MSALIGN__(X)
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4101)
#pragma warning(disable: 4133)
#pragma warning(disable: 4146)
#endif//_MSC_VER


/* Global Declarations */

/* Types Declarations */
struct l_unnamed_1;
struct l_struct____rpsl_node_info_struct;
struct l_struct____rpsl_entry_desc_struct;
struct l_struct____rpsl_type_info_struct;
struct l_struct____rpsl_params_info_struct;
struct l_struct____rpsl_shader_ref_struct;
struct l_struct____rpsl_pipeline_info_struct;
struct l_struct____rpsl_pipeline_field_info_struct;
struct l_struct____rpsl_pipeline_res_binding_info_struct;
struct l_struct____rpsl_module_info_struct;
struct l_struct_struct_OC_RpsTypeInfo;
struct l_struct_struct_OC_RpsParameterDesc;
struct l_struct_struct_OC_RpsNodeDesc;
struct l_struct_struct_OC_RpslEntry;
struct SubresourceRange;
struct texture;
struct ResourceDesc;

/* Function definitions */
typedef void l_fptr_1(uint32_t, uint8_t**, uint32_t);


/* Types Definitions */

/** RPS Built-In Types Begin **/

typedef enum _RpsFormat {
    _RPS_FORMAT_UNKNOWN = 0,
    _RPS_FORMAT_R32G32B32A32_TYPELESS,
    _RPS_FORMAT_R32G32B32A32_FLOAT,
    _RPS_FORMAT_R32G32B32A32_UINT,
    _RPS_FORMAT_R32G32B32A32_SINT,
    _RPS_FORMAT_R32G32B32_TYPELESS,
    _RPS_FORMAT_R32G32B32_FLOAT,
    _RPS_FORMAT_R32G32B32_UINT,
    _RPS_FORMAT_R32G32B32_SINT,
    _RPS_FORMAT_R16G16B16A16_TYPELESS,
    _RPS_FORMAT_R16G16B16A16_FLOAT,
    _RPS_FORMAT_R16G16B16A16_UNORM,
    _RPS_FORMAT_R16G16B16A16_UINT,
    _RPS_FORMAT_R16G16B16A16_SNORM,
    _RPS_FORMAT_R16G16B16A16_SINT,
    _RPS_FORMAT_R32G32_TYPELESS,
    _RPS_FORMAT_R32G32_FLOAT,
    _RPS_FORMAT_R32G32_UINT,
    _RPS_FORMAT_R32G32_SINT,
    _RPS_FORMAT_R32G8X24_TYPELESS,
    _RPS_FORMAT_D32_FLOAT_S8X24_UINT,
    _RPS_FORMAT_R32_FLOAT_X8X24_TYPELESS,
    _RPS_FORMAT_X32_TYPELESS_G8X24_UINT,
    _RPS_FORMAT_R10G10B10A2_TYPELESS,
    _RPS_FORMAT_R10G10B10A2_UNORM,
    _RPS_FORMAT_R10G10B10A2_UINT,
    _RPS_FORMAT_R11G11B10_FLOAT,
    _RPS_FORMAT_R8G8B8A8_TYPELESS,
    _RPS_FORMAT_R8G8B8A8_UNORM,
    _RPS_FORMAT_R8G8B8A8_UNORM_SRGB,
    _RPS_FORMAT_R8G8B8A8_UINT,
    _RPS_FORMAT_R8G8B8A8_SNORM,
    _RPS_FORMAT_R8G8B8A8_SINT,
    _RPS_FORMAT_R16G16_TYPELESS,
    _RPS_FORMAT_R16G16_FLOAT,
    _RPS_FORMAT_R16G16_UNORM,
    _RPS_FORMAT_R16G16_UINT,
    _RPS_FORMAT_R16G16_SNORM,
    _RPS_FORMAT_R16G16_SINT,
    _RPS_FORMAT_R32_TYPELESS,
    _RPS_FORMAT_D32_FLOAT,
    _RPS_FORMAT_R32_FLOAT,
    _RPS_FORMAT_R32_UINT,
    _RPS_FORMAT_R32_SINT,
    _RPS_FORMAT_R24G8_TYPELESS,
    _RPS_FORMAT_D24_UNORM_S8_UINT,
    _RPS_FORMAT_R24_UNORM_X8_TYPELESS,
    _RPS_FORMAT_X24_TYPELESS_G8_UINT,
    _RPS_FORMAT_R8G8_TYPELESS,
    _RPS_FORMAT_R8G8_UNORM,
    _RPS_FORMAT_R8G8_UINT,
    _RPS_FORMAT_R8G8_SNORM,
    _RPS_FORMAT_R8G8_SINT,
    _RPS_FORMAT_R16_TYPELESS,
    _RPS_FORMAT_R16_FLOAT,
    _RPS_FORMAT_D16_UNORM,
    _RPS_FORMAT_R16_UNORM,
    _RPS_FORMAT_R16_UINT,
    _RPS_FORMAT_R16_SNORM,
    _RPS_FORMAT_R16_SINT,
    _RPS_FORMAT_R8_TYPELESS,
    _RPS_FORMAT_R8_UNORM,
    _RPS_FORMAT_R8_UINT,
    _RPS_FORMAT_R8_SNORM,
    _RPS_FORMAT_R8_SINT,
    _RPS_FORMAT_A8_UNORM,
    _RPS_FORMAT_R1_UNORM,
    _RPS_FORMAT_R9G9B9E5_SHAREDEXP,
    _RPS_FORMAT_R8G8_B8G8_UNORM,
    _RPS_FORMAT_G8R8_G8B8_UNORM,
    _RPS_FORMAT_BC1_TYPELESS,
    _RPS_FORMAT_BC1_UNORM,
    _RPS_FORMAT_BC1_UNORM_SRGB,
    _RPS_FORMAT_BC2_TYPELESS,
    _RPS_FORMAT_BC2_UNORM,
    _RPS_FORMAT_BC2_UNORM_SRGB,
    _RPS_FORMAT_BC3_TYPELESS,
    _RPS_FORMAT_BC3_UNORM,
    _RPS_FORMAT_BC3_UNORM_SRGB,
    _RPS_FORMAT_BC4_TYPELESS,
    _RPS_FORMAT_BC4_UNORM,
    _RPS_FORMAT_BC4_SNORM,
    _RPS_FORMAT_BC5_TYPELESS,
    _RPS_FORMAT_BC5_UNORM,
    _RPS_FORMAT_BC5_SNORM,
    _RPS_FORMAT_B5G6R5_UNORM,
    _RPS_FORMAT_B5G5R5A1_UNORM,
    _RPS_FORMAT_B8G8R8A8_UNORM,
    _RPS_FORMAT_B8G8R8X8_UNORM,
    _RPS_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
    _RPS_FORMAT_B8G8R8A8_TYPELESS,
    _RPS_FORMAT_B8G8R8A8_UNORM_SRGB,
    _RPS_FORMAT_B8G8R8X8_TYPELESS,
    _RPS_FORMAT_B8G8R8X8_UNORM_SRGB,
    _RPS_FORMAT_BC6H_TYPELESS,
    _RPS_FORMAT_BC6H_UF16,
    _RPS_FORMAT_BC6H_SF16,
    _RPS_FORMAT_BC7_TYPELESS,
    _RPS_FORMAT_BC7_UNORM,
    _RPS_FORMAT_BC7_UNORM_SRGB,
    _RPS_FORMAT_AYUV,
    _RPS_FORMAT_Y410,
    _RPS_FORMAT_Y416,
    _RPS_FORMAT_NV12,
    _RPS_FORMAT_P010,
    _RPS_FORMAT_P016,
    _RPS_FORMAT_420_OPAQUE,
    _RPS_FORMAT_YUY2,
    _RPS_FORMAT_Y210,
    _RPS_FORMAT_Y216,
    _RPS_FORMAT_NV11,
    _RPS_FORMAT_AI44,
    _RPS_FORMAT_IA44,
    _RPS_FORMAT_P8,
    _RPS_FORMAT_A8P8,
    _RPS_FORMAT_B4G4R4A4_UNORM,

    _RPS_FORMAT_COUNT,

    _RPS_FORMAT_FORCE_INT32 = 0x7FFFFFFF,
} _RpsFormat;

typedef enum _RpsResourceType
{
    _RPS_RESOURCE_TYPE_BUFFER = 0,
    _RPS_RESOURCE_TYPE_IMAGE_1D,
    _RPS_RESOURCE_TYPE_IMAGE_2D,
    _RPS_RESOURCE_TYPE_IMAGE_3D,
    _RPS_RESOURCE_TYPE_UNKNOWN,

    _RPS_RESOURCE_TYPE_FORCE_INT32 = 0x7FFFFFFF,
} _RpsResourceType;

typedef enum _RpsResourceFlags
{
    RPS_RESOURCE_FLAG_NONE                        = 0,
    RPS_RESOURCE_CUBEMAP_COMPATIBLE_BIT           = (1 << 1),
    RPS_RESOURCE_ROWMAJOR_IMAGE_BIT               = (1 << 2),
    RPS_RESOURCE_PREFER_GPU_LOCAL_CPU_VISIBLE_BIT = (1 << 3),
    RPS_RESOURCE_PREFER_DEDICATED_BIT             = (1 << 4),
    RPS_RESOURCE_PERSISTENT_BIT                   = (1 << 15),
} _RpsResourceFlags;


typedef struct SubresourceRange {
  uint16_t base_mip_level;
  uint16_t mip_level_count;
  uint32_t base_array_layer;
  uint32_t array_layer_count;
} SubresourceRange;

typedef struct texture {
  uint32_t Resource;
  _RpsFormat Format;
  uint32_t TemporalLayer;
  uint32_t Flags;
  SubresourceRange SubresourceRange;
  float MinLodClamp;
  uint32_t ComponentMapping;
} texture;

typedef struct buffer {
  uint32_t Resource;
  _RpsFormat Format;
  uint32_t TemporalLayer;
  uint32_t Flags;
  uint64_t Offset;
  uint64_t SizeInBytes;
  uint32_t StructureByteStride;
} buffer;

typedef struct ResourceDesc {
  _RpsResourceType Type;
  uint32_t TemporalLayers;
  _RpsResourceFlags Flags;
  uint32_t Width;
  uint32_t Height;
  uint32_t DepthOrArraySize;
  uint32_t MipLevels;
  _RpsFormat Format;
  uint32_t SampleCount;
} ResourceDesc;

typedef struct RpsViewport {
  float x;
  float y;
  float width;
  float height;
  float minZ;
  float maxZ;
} RpsViewport;

typedef struct ShaderModule {
  uint32_t h;
} ShaderModule;

typedef struct Pipeline {
  uint32_t h;
} Pipeline;
/** RPS Built-In Types End **/

struct l_unnamed_1 {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
};
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_node_info_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
  uint32_t field4;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_entry_desc_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
  uint8_t* field4;
  uint8_t* field5;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_type_info_struct {
  uint8_t field0;
  uint8_t field1;
  uint8_t field2;
  uint8_t field3;
  uint32_t field4;
  uint32_t field5;
  uint32_t field6;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_params_info_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
  uint32_t field4;
  uint16_t field5;
  uint16_t field6;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_shader_ref_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_pipeline_info_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_pipeline_field_info_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
  uint32_t field4;
  uint32_t field5;
  uint32_t field6;
  uint32_t field7;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_pipeline_res_binding_info_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
struct l_array_137_uint8_t {
  uint8_t array[137];
};
struct l_array_4_struct_AC_l_struct____rpsl_node_info_struct {
  struct l_struct____rpsl_node_info_struct array[4];
};
struct l_array_4_struct_AC_l_struct____rpsl_type_info_struct {
  struct l_struct____rpsl_type_info_struct array[4];
};
struct l_array_10_struct_AC_l_struct____rpsl_params_info_struct {
  struct l_struct____rpsl_params_info_struct array[10];
};
struct l_array_3_struct_AC_l_struct____rpsl_entry_desc_struct {
  struct l_struct____rpsl_entry_desc_struct array[3];
};
struct l_array_1_struct_AC_l_struct____rpsl_shader_ref_struct {
  struct l_struct____rpsl_shader_ref_struct array[1];
};
struct l_array_1_struct_AC_l_struct____rpsl_pipeline_info_struct {
  struct l_struct____rpsl_pipeline_info_struct array[1];
};
struct l_array_1_struct_AC_l_struct____rpsl_pipeline_field_info_struct {
  struct l_struct____rpsl_pipeline_field_info_struct array[1];
};
struct l_array_1_struct_AC_l_struct____rpsl_pipeline_res_binding_info_struct {
  struct l_struct____rpsl_pipeline_res_binding_info_struct array[1];
};
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct l_struct____rpsl_module_info_struct {
  uint32_t field0;
  uint32_t field1;
  uint32_t field2;
  uint32_t field3;
  uint32_t field4;
  uint32_t field5;
  uint32_t field6;
  uint32_t field7;
  uint32_t field8;
  uint32_t field9;
  uint32_t field10;
  uint32_t field11;
  uint32_t field12;
  struct l_array_137_uint8_t* field13;
  struct l_array_4_struct_AC_l_struct____rpsl_node_info_struct* field14;
  struct l_array_4_struct_AC_l_struct____rpsl_type_info_struct* field15;
  struct l_array_10_struct_AC_l_struct____rpsl_params_info_struct* field16;
  struct l_array_3_struct_AC_l_struct____rpsl_entry_desc_struct* field17;
  struct l_array_1_struct_AC_l_struct____rpsl_shader_ref_struct* field18;
  struct l_array_1_struct_AC_l_struct____rpsl_pipeline_info_struct* field19;
  struct l_array_1_struct_AC_l_struct____rpsl_pipeline_field_info_struct* field20;
  struct l_array_1_struct_AC_l_struct____rpsl_pipeline_res_binding_info_struct* field21;
  uint32_t field22;
} __attribute__ ((packed));
#ifdef _MSC_VER
#pragma pack(pop)
#endif
struct l_struct_struct_OC_RpsTypeInfo {
  uint16_t field0;
  uint16_t field1;
};
struct l_struct_struct_OC_RpsParameterDesc {
  struct l_struct_struct_OC_RpsTypeInfo field0;
  uint32_t field1;
  struct l_unnamed_1* field2;
  uint8_t* field3;
  uint32_t field4;
};
struct l_struct_struct_OC_RpsNodeDesc {
  uint32_t field0;
  uint32_t field1;
  struct l_struct_struct_OC_RpsParameterDesc* field2;
  uint8_t* field3;
};
struct l_struct_struct_OC_RpslEntry {
  uint8_t* field0;
  l_fptr_1* field1;
  struct l_struct_struct_OC_RpsParameterDesc* field2;
  struct l_struct_struct_OC_RpsNodeDesc* field3;
  uint32_t field4;
  uint32_t field5;
};
struct l_array_12_uint8_t {
  uint8_t array[12];
};
struct l_array_2_uint8_t {
  uint8_t array[2];
};
struct l_array_5_uint8_t {
  uint8_t array[5];
};
struct l_array_2_struct_AC_l_struct_struct_OC_RpsParameterDesc {
  struct l_struct_struct_OC_RpsParameterDesc array[2];
};
struct l_array_9_uint8_t {
  uint8_t array[9];
};
struct l_array_1_struct_AC_l_struct_struct_OC_RpsParameterDesc {
  struct l_struct_struct_OC_RpsParameterDesc array[1];
};
struct l_array_13_uint8_t {
  uint8_t array[13];
};
struct l_array_18_uint8_t {
  uint8_t array[18];
};
struct l_array_19_uint8_t {
  uint8_t array[19];
};
struct l_array_3_struct_AC_l_struct_struct_OC_RpsParameterDesc {
  struct l_struct_struct_OC_RpsParameterDesc array[3];
};
struct l_array_14_uint8_t {
  uint8_t array[14];
};
struct l_array_3_struct_AC_l_struct_struct_OC_RpsNodeDesc {
  struct l_struct_struct_OC_RpsNodeDesc array[3];
};
struct l_array_11_uint8_t {
  uint8_t array[11];
};
struct l_vector_4_float {
  float vector[4];
} __attribute__((aligned(4)));
struct l_array_2_uint8_t_KC_ {
  uint8_t* array[2];
};
struct l_array_3_uint8_t_KC_ {
  uint8_t* array[3];
};

/* External Global Variable Declarations */

/* Function Declarations */
static struct texture _BA__PD_make_default_texture_view_from_desc_AE__AE_YA_PD_AUtexture_AE__AE_IUResourceDesc_AE__AE__AE_Z(uint32_t, struct ResourceDesc*) __ATTRIBUTELIST__((nothrow)) __asm__ ("?make_default_texture_view_from_desc@@YA?AUtexture@@IUResourceDesc@@@Z");
void rpsl_M_hello_triangle_Fn_main(struct texture*) __ATTRIBUTELIST__((nothrow));
void rpsl_M_hello_triangle_Fn_mainBreathing(struct texture*, float) __ATTRIBUTELIST__((nothrow));
void ___rpsl_abort(uint32_t);
uint32_t ___rpsl_node_call(uint32_t, uint32_t, uint8_t**, uint32_t, uint32_t);
void ___rpsl_block_marker(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
void ___rpsl_describe_handle(uint8_t*, uint32_t, uint32_t*, uint32_t);
void rpsl_M_hello_triangle_Fn_main_wrapper(uint32_t, uint8_t**, uint32_t) __ATTRIBUTELIST__((noinline, nothrow));
void rpsl_M_hello_triangle_Fn_mainBreathing_wrapper(uint32_t, uint8_t**, uint32_t) __ATTRIBUTELIST__((noinline, nothrow));


/* Global Variable Definitions and Initialization */
static __MSALIGN__(4) struct l_array_4_struct_AC_l_struct____rpsl_node_info_struct ___rpsl_nodedefs_hello_triangle __attribute__((aligned(4))) = { { { 0, 68, 0, 2, 1 }, { 1, 80, 2, 1, 1 }, { 2, 89, 3, 3, 1 }, { 0, 0, 0, 0, 0 } } };
static __MSALIGN__(4) struct l_array_3_struct_AC_l_struct____rpsl_entry_desc_struct ___rpsl_entries_hello_triangle __attribute__((aligned(4))) = { { { 0, 107, 6, 1, ((uint8_t*)rpsl_M_hello_triangle_Fn_main), ((uint8_t*)rpsl_M_hello_triangle_Fn_main_wrapper) }, { 1, 123, 7, 2, ((uint8_t*)rpsl_M_hello_triangle_Fn_mainBreathing), ((uint8_t*)rpsl_M_hello_triangle_Fn_mainBreathing_wrapper) }, { 0, 0, 0, 0, ((uint8_t*)/*NULL*/0), ((uint8_t*)/*NULL*/0) } } };
static __MSALIGN__(4) struct l_array_4_struct_AC_l_struct____rpsl_type_info_struct ___rpsl_types_metadata_hello_triangle __attribute__((aligned(4))) = { { { 6, 0, 0, 0, 0, 36, 4 }, { 4, 32, 0, 4, 0, 16, 4 }, { 4, 32, 0, 0, 0, 4, 4 }, { 0, 0, 0, 0, 0, 0, 0 } } };
static __MSALIGN__(4) struct l_array_10_struct_AC_l_struct____rpsl_params_info_struct ___rpsl_params_metadata_hello_triangle __attribute__((aligned(4))) = { { { 15, 0, 272629888, -1, 0, 36, 0 }, { 17, 1, 0, -1, 0, 16, 36 }, { 22, 0, 128, -1, 0, 36, 0 }, { 22, 0, 128, -1, 0, 36, 0 }, { 35, 2, 0, -1, 0, 4, 36 }, { 54, 2, 0, -1, 0, 4, 40 }, { 112, 0, 524288, -1, 0, 36, 0 }, { 112, 0, 524288, -1, 0, 36, 0 }, { 54, 2, 0, -1, 0, 4, 36 }, { 0, 0, 0, 0, 0, 0, 0 } } };
static __MSALIGN__(4) struct l_array_1_struct_AC_l_struct____rpsl_shader_ref_struct ___rpsl_shader_refs_hello_triangle __attribute__((aligned(4)));
static __MSALIGN__(4) struct l_array_1_struct_AC_l_struct____rpsl_pipeline_info_struct ___rpsl_pipelines_hello_triangle __attribute__((aligned(4)));
static __MSALIGN__(4) struct l_array_1_struct_AC_l_struct____rpsl_pipeline_field_info_struct ___rpsl_pipeline_fields_hello_triangle __attribute__((aligned(4)));
static __MSALIGN__(4) struct l_array_1_struct_AC_l_struct____rpsl_pipeline_res_binding_info_struct ___rpsl_pipeline_res_bindings_hello_triangle __attribute__((aligned(4)));
__MSALIGN__(4) struct l_array_137_uint8_t ___rpsl_string_table_hello_triangle __attribute__((aligned(4))) = { { 104u, 101u, 108u, 108u, 111u, 95u, 116u, 114u, 105u, 97u, 110u, 103u, 108u, 101u, 0, 116u, 0, 100u, 97u, 116u, 97u, 0, 114u, 101u, 110u, 100u, 101u, 114u, 84u, 97u, 114u, 103u, 101u, 116u, 0, 111u, 110u, 101u, 79u, 118u, 101u, 114u, 65u, 115u, 112u, 101u, 99u, 116u, 82u, 97u, 116u, 105u, 111u, 0, 116u, 105u, 109u, 101u, 73u, 110u, 83u, 101u, 99u, 111u, 110u, 100u, 115u, 0, 99u, 108u, 101u, 97u, 114u, 95u, 99u, 111u, 108u, 111u, 114u, 0, 84u, 114u, 105u, 97u, 110u, 103u, 108u, 101u, 0, 84u, 114u, 105u, 97u, 110u, 103u, 108u, 101u, 66u, 114u, 101u, 97u, 116u, 104u, 105u, 110u, 103u, 0, 109u, 97u, 105u, 110u, 0, 98u, 97u, 99u, 107u, 98u, 117u, 102u, 102u, 101u, 114u, 0, 109u, 97u, 105u, 110u, 66u, 114u, 101u, 97u, 116u, 104u, 105u, 110u, 103u, 0 } };
__CBE_DLLEXPORT__ __MSALIGN__(4) struct l_struct____rpsl_module_info_struct ___rpsl_module_info_hello_triangle __attribute__((aligned(4))) = { 1297305682u, 3, 9, 0, 137, 3, 3, 9, 2, 0, 0, 0, 0, (&___rpsl_string_table_hello_triangle), (&___rpsl_nodedefs_hello_triangle), (&___rpsl_types_metadata_hello_triangle), (&___rpsl_params_metadata_hello_triangle), (&___rpsl_entries_hello_triangle), (&___rpsl_shader_refs_hello_triangle), (&___rpsl_pipelines_hello_triangle), (&___rpsl_pipeline_fields_hello_triangle), (&___rpsl_pipeline_res_bindings_hello_triangle), 1297305682u };
static struct l_array_12_uint8_t _AE__AE_rps_Str0 = { "clear_color" };
static struct l_array_2_uint8_t _AE__AE_rps_Str1 = { "t" };
static struct l_unnamed_1 _AE__AE_rps_ParamAttr2 = { 272629888, 0, 0, 0 };
static struct l_array_5_uint8_t _AE__AE_rps_Str3 = { "data" };
static struct l_unnamed_1 _AE__AE_rps_ParamAttr4 = { 0, 0, 27, 0 };
static struct l_array_2_struct_AC_l_struct_struct_OC_RpsParameterDesc _AE__AE_rps_ParamDescArray5 = { { { { 36, 64 }, 0, (&_AE__AE_rps_ParamAttr2), ((&_AE__AE_rps_Str1.array[((int32_t)0)])), 4 }, { { 16, 0 }, 0, (&_AE__AE_rps_ParamAttr4), ((&_AE__AE_rps_Str3.array[((int32_t)0)])), 0 } } };
static struct l_array_9_uint8_t _AE__AE_rps_Str6 = { "Triangle" };
static struct l_unnamed_1 _AE__AE_rps_ParamAttr8 = { 128, 0, 35, 0 };
static struct l_array_18_uint8_t _AE__AE_rps_Str10 = { "TriangleBreathing" };
static struct l_array_13_uint8_t _AE__AE_rps_Str11 = { "renderTarget" };
static struct l_array_1_struct_AC_l_struct_struct_OC_RpsParameterDesc _AE__AE_rps_ParamDescArray9 = { { { { 36, 64 }, 0, (&_AE__AE_rps_ParamAttr8), ((&_AE__AE_rps_Str11.array[((int32_t)0)])), 4 } } };
static struct l_unnamed_1 _AE__AE_rps_ParamAttr12 = { 128, 0, 35, 0 };
static struct l_array_19_uint8_t _AE__AE_rps_Str13 = { "oneOverAspectRatio" };
static struct l_unnamed_1 _AE__AE_rps_ParamAttr14;
static struct l_unnamed_1 _AE__AE_rps_ParamAttr16;
static struct l_array_5_uint8_t _AE__AE_rps_Str18 = { "main" };
static struct l_unnamed_1 _AE__AE_rps_ParamAttr20 = { 524288, 0, 0, 0 };
static struct l_array_14_uint8_t _AE__AE_rps_Str22 = { "mainBreathing" };
static struct l_array_11_uint8_t _AE__AE_rps_Str23 = { "backbuffer" };
static struct l_array_1_struct_AC_l_struct_struct_OC_RpsParameterDesc _AE__AE_rps_ParamDescArray21 = { { { { 36, 64 }, 0, (&_AE__AE_rps_ParamAttr20), ((&_AE__AE_rps_Str23.array[((int32_t)0)])), 4 } } };
static struct l_unnamed_1 _AE__AE_rps_ParamAttr24 = { 524288, 0, 0, 0 };
static struct l_array_14_uint8_t _AE__AE_rps_Str25 = { "timeInSeconds" };
static struct l_array_3_struct_AC_l_struct_struct_OC_RpsParameterDesc _AE__AE_rps_ParamDescArray17 = { { { { 36, 64 }, 0, (&_AE__AE_rps_ParamAttr12), ((&_AE__AE_rps_Str11.array[((int32_t)0)])), 4 }, { { 4, 0 }, 0, (&_AE__AE_rps_ParamAttr14), ((&_AE__AE_rps_Str13.array[((int32_t)0)])), 0 }, { { 4, 0 }, 0, (&_AE__AE_rps_ParamAttr16), ((&_AE__AE_rps_Str25.array[((int32_t)0)])), 0 } } };
__CBE_DLLEXPORT__ struct l_array_3_struct_AC_l_struct_struct_OC_RpsNodeDesc NodeDecls_hello_triangle = { { { 1, 2, ((&_AE__AE_rps_ParamDescArray5.array[((int32_t)0)])), ((&_AE__AE_rps_Str0.array[((int32_t)0)])) }, { 1, 1, ((&_AE__AE_rps_ParamDescArray9.array[((int32_t)0)])), ((&_AE__AE_rps_Str6.array[((int32_t)0)])) }, { 1, 3, ((&_AE__AE_rps_ParamDescArray17.array[((int32_t)0)])), ((&_AE__AE_rps_Str10.array[((int32_t)0)])) } } };
struct l_struct_struct_OC_RpslEntry rpsl_M_hello_triangle_E_main_AE_value = { ((&_AE__AE_rps_Str18.array[((int32_t)0)])), rpsl_M_hello_triangle_Fn_main_wrapper, ((&_AE__AE_rps_ParamDescArray21.array[((int32_t)0)])), ((&NodeDecls_hello_triangle.array[((int32_t)0)])), 1, 3 };
__CBE_DLLEXPORT__ struct l_struct_struct_OC_RpslEntry* rpsl_M_hello_triangle_E_main = (&rpsl_M_hello_triangle_E_main_AE_value);
__CBE_DLLEXPORT__ struct l_struct_struct_OC_RpslEntry** rpsl_M_hello_triangle_E_main_pp = (&rpsl_M_hello_triangle_E_main);
static struct l_unnamed_1 _AE__AE_rps_ParamAttr26;
static struct l_array_2_struct_AC_l_struct_struct_OC_RpsParameterDesc _AE__AE_rps_ParamDescArray27 = { { { { 36, 64 }, 0, (&_AE__AE_rps_ParamAttr24), ((&_AE__AE_rps_Str23.array[((int32_t)0)])), 4 }, { { 4, 0 }, 0, (&_AE__AE_rps_ParamAttr26), ((&_AE__AE_rps_Str25.array[((int32_t)0)])), 0 } } };
struct l_struct_struct_OC_RpslEntry rpsl_M_hello_triangle_E_mainBreathing_AE_value = { ((&_AE__AE_rps_Str22.array[((int32_t)0)])), rpsl_M_hello_triangle_Fn_mainBreathing_wrapper, ((&_AE__AE_rps_ParamDescArray27.array[((int32_t)0)])), ((&NodeDecls_hello_triangle.array[((int32_t)0)])), 2, 3 };
__CBE_DLLEXPORT__ struct l_struct_struct_OC_RpslEntry* rpsl_M_hello_triangle_E_mainBreathing = (&rpsl_M_hello_triangle_E_mainBreathing_AE_value);
__CBE_DLLEXPORT__ struct l_struct_struct_OC_RpslEntry** rpsl_M_hello_triangle_E_mainBreathing_pp = (&rpsl_M_hello_triangle_E_mainBreathing);


/* LLVM Intrinsic Builtin Function Bodies */
static __forceinline float llvm_fdiv_f32(float a, float b) {
  float r = a / b;
  return r;
}
static __forceinline struct l_vector_4_float llvm_ctor_f32x4(float x0, float x1, float x2, float x3) {
  __MSALIGN__(4) struct l_vector_4_float r;
  r.vector[0] = x0;
  r.vector[1] = x1;
  r.vector[2] = x2;
  r.vector[3] = x3;
  return r;
}


/* Function Bodies */

#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
static struct texture _BA__PD_make_default_texture_view_from_desc_AE__AE_YA_PD_AUtexture_AE__AE_IUResourceDesc_AE__AE__AE_Z(uint32_t resourceHdl, struct ResourceDesc* desc) {
  struct texture StructReturn;  /* Struct return temporary */
  struct texture*agg_2e_result = &StructReturn;

  struct {
    uint32_t _1;
    uint32_t _2;
    uint32_t _3;
  } _llvm_cbe_tmps;

  struct {
    uint32_t cond;
    uint32_t cond__PHI_TEMPORARY;
  } _llvm_cbe_phi_tmps = {0};

  #line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  _llvm_cbe_tmps._1 = *((&desc->MipLevels));
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  _llvm_cbe_tmps._2 = *((&desc->Type));
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  if ((((_llvm_cbe_tmps._2 == 4u)&1))) {
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
    _llvm_cbe_phi_tmps.cond__PHI_TEMPORARY = 1;   /* for PHI node */
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
    goto cond_2e_end;
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  } else {
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
    goto cond_2e_false;
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  }

cond_2e_false:
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  _llvm_cbe_tmps._3 = *((&desc->DepthOrArraySize));
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  _llvm_cbe_phi_tmps.cond__PHI_TEMPORARY = _llvm_cbe_tmps._3;   /* for PHI node */
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  goto cond_2e_end;
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"

cond_2e_end:
#line 151 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  _llvm_cbe_phi_tmps.cond = _llvm_cbe_phi_tmps.cond__PHI_TEMPORARY;
  #line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->Resource)) = resourceHdl;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->Format)) = 0;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->TemporalLayer)) = 0;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->Flags)) = 0;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->SubresourceRange.base_mip_level)) = 0;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->SubresourceRange.mip_level_count)) = (((uint16_t)_llvm_cbe_tmps._1));
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->SubresourceRange.base_array_layer)) = 0;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->SubresourceRange.array_layer_count)) = _llvm_cbe_phi_tmps.cond;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->MinLodClamp)) = ((float)(0.000000e+00));
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  *((&agg_2e_result->ComponentMapping)) = 50462976;
#line 153 "./External/RenderPipelineShaders/examples/___rpsl_builtin_header_.rpsl"
  return StructReturn;
}


#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
void rpsl_M_hello_triangle_Fn_main(struct texture* backbuffer) {

  struct {
    struct l_array_2_uint8_t_KC_ _4;    /* Address-exposed local */
    uint8_t** _2e_sub;
    __MSALIGN__(4) struct l_vector_4_float _5;    /* Address-exposed local */
    uint32_t _6;
    uint8_t* _7;    /* Address-exposed local */
    uint32_t _8;
  } _llvm_cbe_tmps;

  struct {
    char _placeholder;
  } _llvm_cbe_phi_tmps = {0};

(void)_llvm_cbe_phi_tmps;

#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  ___rpsl_block_marker(0, 0, 0, 2, -1, 0, -1);
  #line 13 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._2e_sub = (&_llvm_cbe_tmps._4.array[((int32_t)0)]);
#line 13 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((struct texture**)_llvm_cbe_tmps._2e_sub)) = backbuffer;
#line 13 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._5 = llvm_ctor_f32x4(((float)(0.000000e+00)), ((float)(2.000000e-01)), ((float)(4.000000e-01)), ((float)(1.000000e+00)));
#line 13 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((__MSALIGN__(4) struct l_vector_4_float**)((&_llvm_cbe_tmps._4.array[((int32_t)1)])))) = (&_llvm_cbe_tmps._5);
#line 13 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._6 = ___rpsl_node_call(0, 2, _llvm_cbe_tmps._2e_sub, 0, 0);
#line 14 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((struct texture**)(&_llvm_cbe_tmps._7))) = backbuffer;
#line 14 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._8 = ___rpsl_node_call(1, 1, (&_llvm_cbe_tmps._7), 0, 1);
#line 15 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
}


#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
void rpsl_M_hello_triangle_Fn_mainBreathing(struct texture* backbuffer, float timeInSeconds) {
    struct ResourceDesc backbufferDesc;    /* Address-exposed local */

  struct {
    __MSALIGN__(8) struct texture _9 __attribute__((aligned(8)));    /* Address-exposed local */
    uint32_t* _10;
    uint32_t _11;
    uint32_t _12;
    uint32_t _13;
    uint32_t _14;
    uint16_t _15;
    uint16_t _16;
    uint32_t _17;
    uint32_t _18;
    float _19;
    uint32_t _20;
    uint32_t _21;
    uint32_t _22;
    struct l_array_2_uint8_t_KC_ _23;    /* Address-exposed local */
    uint8_t** _2e_sub;
    __MSALIGN__(4) struct l_vector_4_float _24;    /* Address-exposed local */
    uint32_t _25;
    struct l_array_3_uint8_t_KC_ _26;    /* Address-exposed local */
    uint8_t** _2e_sub_2e_3;
    float _27;    /* Address-exposed local */
    float _28;    /* Address-exposed local */
    uint32_t _29;
  } _llvm_cbe_tmps;

  struct {
    char _placeholder;
  } _llvm_cbe_phi_tmps = {0};

(void)_llvm_cbe_phi_tmps;

#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  ___rpsl_block_marker(0, 0, 0, 2, -1, 0, -1);
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._10 = (&_llvm_cbe_tmps._9.Resource);
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._11 = *((&backbuffer->Resource));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *_llvm_cbe_tmps._10 = _llvm_cbe_tmps._11;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._12 = *((&backbuffer->Format));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.Format)) = _llvm_cbe_tmps._12;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._13 = *((&backbuffer->TemporalLayer));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.TemporalLayer)) = _llvm_cbe_tmps._13;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._14 = *((&backbuffer->Flags));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.Flags)) = _llvm_cbe_tmps._14;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._15 = *((&backbuffer->SubresourceRange.base_mip_level));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.SubresourceRange.base_mip_level)) = _llvm_cbe_tmps._15;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._16 = *((&backbuffer->SubresourceRange.mip_level_count));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.SubresourceRange.mip_level_count)) = _llvm_cbe_tmps._16;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._17 = *((&backbuffer->SubresourceRange.base_array_layer));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.SubresourceRange.base_array_layer)) = _llvm_cbe_tmps._17;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._18 = *((&backbuffer->SubresourceRange.array_layer_count));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.SubresourceRange.array_layer_count)) = _llvm_cbe_tmps._18;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._19 = *((&backbuffer->MinLodClamp));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.MinLodClamp)) = _llvm_cbe_tmps._19;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._20 = *((&backbuffer->ComponentMapping));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *((&_llvm_cbe_tmps._9.ComponentMapping)) = _llvm_cbe_tmps._20;
  #line 23 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  ___rpsl_describe_handle((((uint8_t*)(&backbufferDesc))), 36, _llvm_cbe_tmps._10, 1);
#line 25 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._21 = *((&backbufferDesc.Width));
  #line 26 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._22 = *((&backbufferDesc.Height));
  #line 28 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  #line 31 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._2e_sub = (&_llvm_cbe_tmps._23.array[((int32_t)0)]);
#line 31 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((struct texture**)_llvm_cbe_tmps._2e_sub)) = (&_llvm_cbe_tmps._9);
#line 31 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._24 = llvm_ctor_f32x4(((float)(0.000000e+00)), ((float)(2.000000e-01)), ((float)(4.000000e-01)), ((float)(1.000000e+00)));
#line 31 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((__MSALIGN__(4) struct l_vector_4_float**)((&_llvm_cbe_tmps._23.array[((int32_t)1)])))) = (&_llvm_cbe_tmps._24);
#line 31 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._25 = ___rpsl_node_call(0, 2, _llvm_cbe_tmps._2e_sub, 0, 0);
#line 32 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._2e_sub_2e_3 = (&_llvm_cbe_tmps._26.array[((int32_t)0)]);
#line 32 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((struct texture**)_llvm_cbe_tmps._2e_sub_2e_3)) = (&_llvm_cbe_tmps._9);
#line 32 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._27 = (llvm_fdiv_f32((((float)(uint32_t)_llvm_cbe_tmps._22)), (((float)(uint32_t)_llvm_cbe_tmps._21))));
#line 32 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((float**)((&_llvm_cbe_tmps._26.array[((int32_t)1)])))) = (&_llvm_cbe_tmps._27);
#line 32 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._28 = timeInSeconds;
#line 32 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(((float**)((&_llvm_cbe_tmps._26.array[((int32_t)2)])))) = (&_llvm_cbe_tmps._28);
#line 32 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._29 = ___rpsl_node_call(2, 3, _llvm_cbe_tmps._2e_sub_2e_3, 0, 1);
#line 33 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
}


#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
void rpsl_M_hello_triangle_Fn_main_wrapper(uint32_t llvm_cbe_temp__30, uint8_t** llvm_cbe_temp__31, uint32_t llvm_cbe_temp__32) {

  struct {
    uint8_t* _33;
    struct texture* _34;
    struct texture* _35;
  } _llvm_cbe_tmps;

  struct {
    struct texture* _36;
    struct texture* _36__PHI_TEMPORARY;
  } _llvm_cbe_phi_tmps = {0};

#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  if ((((llvm_cbe_temp__30 == 1u)&1))) {
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto trunk;
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  } else {
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto err;
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  }

trunk:
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._33 = *llvm_cbe_temp__31;
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._34 = (struct texture*) alloca(sizeof(struct texture));
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._35 = ((struct texture*)_llvm_cbe_tmps._33);
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  if (((((llvm_cbe_temp__32 & 1) == 0u)&1))) {
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto _2e_preheader;
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  } else {
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    _llvm_cbe_phi_tmps._36__PHI_TEMPORARY = _llvm_cbe_tmps._35;   /* for PHI node */
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto _2e_loopexit;
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  }

err:
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  ___rpsl_abort(-3);
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  return;
_2e_preheader:
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(_llvm_cbe_tmps._34) = _BA__PD_make_default_texture_view_from_desc_AE__AE_YA_PD_AUtexture_AE__AE_IUResourceDesc_AE__AE__AE_Z(0, (((struct ResourceDesc*)_llvm_cbe_tmps._33)));
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_phi_tmps._36__PHI_TEMPORARY = _llvm_cbe_tmps._34;   /* for PHI node */
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  goto _2e_loopexit;
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"

_2e_loopexit:
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_phi_tmps._36 = _llvm_cbe_phi_tmps._36__PHI_TEMPORARY;
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  rpsl_M_hello_triangle_Fn_main(_llvm_cbe_phi_tmps._36);
#line 10 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
}


#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
void rpsl_M_hello_triangle_Fn_mainBreathing_wrapper(uint32_t llvm_cbe_temp__37, uint8_t** llvm_cbe_temp__38, uint32_t llvm_cbe_temp__39) {

  struct {
    uint8_t* _40;
    struct texture* _41;
    struct texture* _42;
    uint8_t* _43;
    float _44;
  } _llvm_cbe_tmps;

  struct {
    struct texture* _45;
    struct texture* _45__PHI_TEMPORARY;
  } _llvm_cbe_phi_tmps = {0};

#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  if ((((llvm_cbe_temp__37 == 2u)&1))) {
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto trunk;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  } else {
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto err;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  }

trunk:
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._40 = *llvm_cbe_temp__38;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._41 = (struct texture*) alloca(sizeof(struct texture));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._42 = ((struct texture*)_llvm_cbe_tmps._40);
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  if (((((llvm_cbe_temp__39 & 1) == 0u)&1))) {
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto _2e_preheader;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  } else {
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    _llvm_cbe_phi_tmps._45__PHI_TEMPORARY = _llvm_cbe_tmps._42;   /* for PHI node */
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
    goto _2e_loopexit;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  }

err:
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  ___rpsl_abort(-3);
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  return;
_2e_preheader:
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  *(_llvm_cbe_tmps._41) = _BA__PD_make_default_texture_view_from_desc_AE__AE_YA_PD_AUtexture_AE__AE_IUResourceDesc_AE__AE__AE_Z(0, (((struct ResourceDesc*)_llvm_cbe_tmps._40)));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_phi_tmps._45__PHI_TEMPORARY = _llvm_cbe_tmps._41;   /* for PHI node */
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  goto _2e_loopexit;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"

_2e_loopexit:
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_phi_tmps._45 = _llvm_cbe_phi_tmps._45__PHI_TEMPORARY;
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._43 = *((&llvm_cbe_temp__38[((int32_t)1)]));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  _llvm_cbe_tmps._44 = *(((float*)_llvm_cbe_tmps._43));
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
  rpsl_M_hello_triangle_Fn_mainBreathing(_llvm_cbe_phi_tmps._45, _llvm_cbe_tmps._44);
#line 21 "./External/RenderPipelineShaders/examples/hello_triangle.rpsl"
}

