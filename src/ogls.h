#include <stdint.h>

#define OGLS_CHECK_ERROR() ::ogls::printErrorCodeMsg(__FILE__, __LINE__)

enum OglsResult
{
	Ogls_Result_Failed  = 0,
	Ogls_Result_Success = 1,
};

enum OglsDataType
{
	Ogls_DataType_Byte,
	Ogls_DataType_UnsignedByte,
	Ogls_DataType_Short,
	Ogls_DataType_UnsignedShort,
	Ogls_DataType_Int,
	Ogls_DataType_UnsignedInt,
	Ogls_DataType_Float,
	Ogls_DataType_Double,
};

struct OglsVertexBuffer;
struct OglsIndexBuffer;
struct OglsVertexArray;
struct OglsVertexArrayCreateInfo;
struct OglsVertexArrayAttribute;
struct OglsShader;
struct OglsShaderCreateInfo;


namespace ogls
{
	OglsResult printErrorCodeMsg(const char* file, int line);
	OglsResult createVertexBuffer(OglsVertexBuffer** vertexBuffer, float* vertices, uint32_t count);
	OglsResult createIndexBuffer(OglsIndexBuffer** indexBuffer, uint32_t* indices, uint32_t count);
	OglsResult createVertexArray(OglsVertexArray** vertexArray, OglsVertexArrayCreateInfo* createInfo);
	OglsResult createShader(OglsShader** shader, OglsShaderCreateInfo* pathToShaderFiles);
	OglsResult createShaderFromStr(OglsShader** shader, OglsShaderCreateInfo* shaderStrings);

	float*     getVertexBufferVertices(OglsVertexBuffer* vertexBuffer);
	uint32_t   getVertexBufferCount(OglsVertexBuffer* vertexBuffer);
	uint32_t   getVertexBufferSize(OglsVertexBuffer* vertexBuffer);
	uint32_t   getVertexBufferId(OglsVertexBuffer* vertexBuffer);

	uint32_t*  getIndexBufferIndices(OglsIndexBuffer* indexBuffer);
	uint32_t   getIndexBufferCount(OglsIndexBuffer* indexBuffer);
	uint32_t   getIndexBufferSize(OglsIndexBuffer* indexBuffer);
	uint32_t   getIndexBufferId(OglsIndexBuffer* indexBuffer);

	uint32_t   getVertexBufferId(OglsVertexArray* vertexArray);
	uint32_t   getVertexBufferId(OglsShader* shader);


	void       bindVertexBuffer(OglsVertexBuffer* vertexBuffer);
	void       bindIndexBuffer(OglsIndexBuffer* indexBuffer);
	void       bindVertexArray(OglsVertexArray* vertexArray);
	void       bindShader(OglsShader* shader);

	void       destroyVertexBuffer(OglsVertexBuffer* vertexBuffer);
	void       destroyIndexBuffer(OglsIndexBuffer* indexBuffer);
	void       destroyVertexArray(OglsVertexArray* vertexArray);
	void       destroyShader(OglsShader* shader);

	void       renderDraw(uint32_t first, uint32_t count);
	void       renderDrawIndex(uint32_t count);
}

struct OglsVertexArrayAttribute
{
	uint32_t index;
	uint32_t components;
	uint32_t stride;
	OglsDataType dataType;
	void* offset;
};

struct OglsVertexArrayCreateInfo
{
	OglsVertexBuffer* vertexBuffer;
	OglsIndexBuffer* indexBuffer;
	OglsVertexArrayAttribute* pAttributes;
	uint32_t attributeCount;
};

struct OglsShaderCreateInfo
{
	const char* vertexSrc;
	const char* fragmentSrc;
};
