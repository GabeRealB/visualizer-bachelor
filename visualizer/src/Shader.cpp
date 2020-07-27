#include <visualizer/Shader.hpp>

#include <charconv>
#include <fstream>
#include <iostream>
#include <utility>

namespace Visualizer {

static std::vector<std::string_view> splitSV(std::string_view strv, std::string_view delims = " "sv)
{
    std::vector<std::string_view> output;
    size_t first = 0;

    while (first < strv.size()) {
        const auto second = strv.find_first_of(delims, first);

        if (first != second)
            output.emplace_back(strv.substr(first, second - first));

        if (second == std::string_view::npos)
            break;

        first = second + 1;
    }

    return output;
}

template <typename T> T fromStringView(std::string_view stringView)
{
    T tmp{};
    std::from_chars(stringView.data(), stringView.data() + stringView.size(), tmp);
    return tmp;
}

/**************************************************************************************************
 ********************************************* Shader *********************************************
 **************************************************************************************************/

constexpr std::array<std::string_view, 2> sParameterQualifierNames{ "@program"sv, "@material"sv };
constexpr std::array<ParameterQualifier, 2> sParameterQualifierMap{ ParameterQualifier::Program,
    ParameterQualifier::Material };

constexpr std::array<std::string_view, 26> sParameterTypeNames{
    "bool"sv,
    "int"sv,
    "uint"sv,
    "float"sv,
    "bvec2"sv,
    "bvec3"sv,
    "bvec4"sv,
    "ivec2"sv,
    "ivec3"sv,
    "ivec4"sv,
    "uvec2"sv,
    "uvec3"sv,
    "uvec4"sv,
    "vec2"sv,
    "vec3"sv,
    "vec4"sv,
    "mat2x2"sv,
    "mat2x3"sv,
    "mat2x4"sv,
    "mat3x2"sv,
    "mat3x3"sv,
    "mat3x4"sv,
    "mat4x2"sv,
    "mat4x3"sv,
    "mat4x4"sv,
    "sampler2D"sv,
};

Shader::Shader(const std::filesystem::path& shaderPath, ShaderType shaderType)
    : m_shader{ 0 }
    , m_shaderType{ shaderType }
    , m_parameters{}
{
    if (!std::filesystem::exists(shaderPath)) {
        return;
    }

    std::string shaderFile{};
    std::ifstream shaderStream{ shaderPath };
    shaderStream.seekg(0, std::ios::end);
    auto shaderSize{ static_cast<std::size_t>(shaderStream.tellg()) };
    shaderStream.seekg(0, std::ios::beg);
    shaderFile.reserve(shaderSize);

    std::string line{};
    while (std::getline(shaderStream, line)) {
        auto lineTokens{ splitSV({ line.data(), line.size() }, " ") };
        if (lineTokens.size() >= 4) {
            if (auto qualPos{
                    std::find(sParameterQualifierNames.begin(), sParameterQualifierNames.end(), lineTokens[0]) };
                qualPos != sParameterQualifierNames.end()) {
                auto qualifier{ sParameterQualifierMap[qualPos - sParameterQualifierNames.begin()] };

                if (auto typePos{ std::find(sParameterTypeNames.begin(), sParameterTypeNames.end(), lineTokens[1]) };
                    typePos != sParameterTypeNames.end()) {
                    auto type{ static_cast<ParameterType>(typePos - sParameterTypeNames.begin()) };
                    auto arraySize{ fromStringView<std::size_t>(lineTokens[2]) };
                    auto parameterName{ lineTokens[3] };

                    m_parameters.emplace_back(
                        qualifier, type, arraySize, std::string{ parameterName.data(), parameterName.size() });
                    continue;
                }
            }
        }
        shaderFile.append("\n");
        shaderFile.append(line);
    }

    switch (m_shaderType) {
    case ShaderType::VertexShader:
        m_shader = glCreateShader(GL_VERTEX_SHADER);
        break;
    case ShaderType::FragmentShader:
        m_shader = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    default:
        m_parameters.clear();
        return;
    }

    auto shaderDataPtr{ shaderFile.data() };
    glShaderSource(m_shader, 1, &shaderDataPtr, nullptr);
    glCompileShader(m_shader);
    GLint testVal;
    glGetShaderiv(m_shader, GL_COMPILE_STATUS, &testVal);
    if (testVal == GL_FALSE) {
        char infoLog[1024];
        glGetShaderInfoLog(m_shader, 1024, NULL, infoLog);
        std::cerr << "The vertex shader failed to compile with the error:" << std::endl << infoLog << std::endl;
        glDeleteShader(m_shader);
        m_shader = 0;
        m_parameters.clear();
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_shader{ std::exchange(other.m_shader, 0) }
    , m_shaderType{ other.m_shaderType }
    , m_parameters{ std::exchange(other.m_parameters, {}) }
{
}

Shader::~Shader()
{
    if (m_shader != 0) {
        glDeleteShader(m_shader);
    }
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other) {
        if (m_shader != 0) {
            glDeleteShader(m_shader);
        }

        m_shader = std::exchange(other.m_shader, 0);
        m_shaderType = other.m_shaderType;
        m_parameters = std::exchange(other.m_parameters, {});
    }
    return *this;
}

GLuint Shader::shader() const { return m_shader; }
ShaderType Shader::shaderType() const { return m_shaderType; }
std::span<const ParameterDeclaration> Shader::parameters() const
{
    return { m_parameters.data(), m_parameters.size() };
}

std::optional<Shader> Shader::create(const std::filesystem::path& shaderPath, ShaderType shaderType)
{
    Shader shader{ shaderPath, shaderType };
    if (shader.m_shader != 0) {
        return shader;
    } else {
        return std::nullopt;
    }
}

/**************************************************************************************************
 *************************************** ShaderEnvironment ***************************************
 **************************************************************************************************/

constexpr std::array<std::tuple<std::size_t, std::size_t>, 26> sTypeSizeAlignmentPairs{
    std::tuple<std::size_t, std::size_t>{ sizeof(GLboolean), alignof(GLboolean) },
    std::tuple<std::size_t, std::size_t>{ sizeof(GLint), alignof(GLint) },
    std::tuple<std::size_t, std::size_t>{ sizeof(GLuint), alignof(GLuint) },
    std::tuple<std::size_t, std::size_t>{ sizeof(GLfloat), alignof(GLfloat) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::bvec2), alignof(glm::bvec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::bvec3), alignof(glm::bvec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::bvec4), alignof(glm::bvec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::ivec2), alignof(glm::ivec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::ivec3), alignof(glm::ivec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::ivec4), alignof(glm::ivec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::uvec2), alignof(glm::uvec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::uvec3), alignof(glm::uvec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::uvec4), alignof(glm::uvec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::vec2), alignof(glm::vec2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::vec3), alignof(glm::vec3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::vec4), alignof(glm::vec4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat2x2), alignof(glm::mat2x2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat2x3), alignof(glm::mat2x3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat2x4), alignof(glm::mat2x4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat3x2), alignof(glm::mat3x2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat3x3), alignof(glm::mat3x3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat3x4), alignof(glm::mat3x4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat4x2), alignof(glm::mat4x2) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat4x3), alignof(glm::mat4x3) },
    std::tuple<std::size_t, std::size_t>{ sizeof(glm::mat4x4), alignof(glm::mat4x4) },
    std::tuple<std::size_t, std::size_t>{ sizeof(TextureSampler<Texture2D>), alignof(TextureSampler<Texture2D>) },
};

ShaderEnvironment::ShaderEnvironment(ShaderProgram& program, ParameterQualifier filter)
    : m_dataSize{ 0 }
    , m_dataAlignment{ 0 }
    , m_parameterNames{}
    , m_parameterInfos{}
    , m_parameterData{}
{
    std::size_t currentPos{ 0 };

    for (auto& parameter : program.parameters()) {
        auto qualifier{ std::get<0>(parameter) };
        auto type{ std::get<1>(parameter) };
        auto size{ std::get<2>(parameter) };
        auto& name{ std::get<3>(parameter) };

        if ((static_cast<std::size_t>(qualifier) & static_cast<std::size_t>(filter)) == 0) {
            continue;
        }

        auto it{ m_parameterInfos.insert_or_assign(name, ShaderEnvironment::ParameterInfo{ currentPos, size, type }) };
        m_parameterNames.emplace_back(it.first->first);

        if (static_cast<std::size_t>(type) <= static_cast<std::size_t>(ParameterType::MaxIndex)) {
            auto sizeAlignmentPair{ sTypeSizeAlignmentPairs[static_cast<std::size_t>(type)] };
            auto typeSize{ std::get<0>(sizeAlignmentPair) };
            auto typeAlignment{ std::get<1>(sizeAlignmentPair) };

            if (m_dataAlignment == 0) {
                m_dataAlignment = typeAlignment;
            }

            currentPos += (currentPos % typeAlignment) + (typeSize * size);
        }
    }

    m_dataSize = currentPos;

    m_parameterData
        = { AlignedDeleter<unsigned char>::allocate(m_dataAlignment, m_dataSize), AlignedDeleter<unsigned char>{} };
    std::memset(m_parameterData.get(), 0, m_dataSize);
}

ShaderEnvironment::ShaderEnvironment(const ShaderEnvironment& other)
    : m_dataSize{ other.m_dataSize }
    , m_dataAlignment{ other.m_dataAlignment }
    , m_parameterNames{}
    , m_parameterInfos{ other.m_parameterInfos }
    , m_parameterData{ AlignedDeleter<unsigned char>::allocate(other.m_dataAlignment, other.m_dataSize),
        AlignedDeleter<unsigned char>{} }
{
    m_parameterNames.reserve(other.m_parameterNames.size());
    for (auto& it : m_parameterInfos) {
        m_parameterNames.emplace_back(it.first);
    }
}

ShaderEnvironment& ShaderEnvironment::operator=(const ShaderEnvironment& other)
{
    if (this != &other) {
        m_dataSize = other.m_dataSize;
        m_parameterInfos = other.m_parameterInfos;
        m_dataAlignment = other.m_dataAlignment;
        m_parameterData = { AlignedDeleter<unsigned char>::allocate(other.m_dataAlignment, other.m_dataSize),
            AlignedDeleter<unsigned char>{} };
        std::memset(m_parameterData.get(), 0, m_dataSize);

        m_parameterNames.clear();
        m_parameterNames.resize(other.m_parameterNames.size());
        for (auto& it : m_parameterInfos) {
            m_parameterNames.emplace_back(it.first);
        }
    }
    return *this;
}

std::span<std::string_view> ShaderEnvironment::parameters() const
{
    return { const_cast<std::string_view*>(m_parameterNames.data()), m_parameterNames.size() };
}

/**************************************************************************************************
 ***************************************** ShaderProgram *****************************************
 **************************************************************************************************/

constexpr std::array<void (*)(const ShaderEnvironment&, GLuint, std::string_view, std::size_t), 26> sTypeApplyFuncs{
    /**************************************** Scalars ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLboolean>(name, size) };
        if (val != nullptr) {
            glUniform1iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLint>(name, size) };
        if (val != nullptr) {
            glUniform1iv(location, static_cast<GLsizei>(size), val);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLuint>(name, size) };
        if (val != nullptr) {
            glUniform1uiv(location, static_cast<GLsizei>(size), val);
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<GLfloat>(name, size) };
        if (val != nullptr) {
            glUniform1fv(location, static_cast<GLsizei>(size), val);
        }
    },
    /**************************************** BVecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::bvec2>(name, size) };
        if (val != nullptr) {
            glUniform2iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(glm::value_ptr(*val)));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::bvec3>(name, size) };
        if (val != nullptr) {
            glUniform3iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(glm::value_ptr(*val)));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::bvec4>(name, size) };
        if (val != nullptr) {
            glUniform4iv(location, static_cast<GLsizei>(size), reinterpret_cast<GLint*>(glm::value_ptr(*val)));
        }
    },
    /**************************************** IVecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::ivec2>(name, size) };
        if (val != nullptr) {
            glUniform2iv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::ivec3>(name, size) };
        if (val != nullptr) {
            glUniform3iv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::ivec4>(name, size) };
        if (val != nullptr) {
            glUniform4iv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    /**************************************** UVecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::uvec2>(name, size) };
        if (val != nullptr) {
            glUniform2uiv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::uvec3>(name, size) };
        if (val != nullptr) {
            glUniform3uiv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::uvec4>(name, size) };
        if (val != nullptr) {
            glUniform4uiv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    /**************************************** VecN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::vec2>(name, size) };
        if (val != nullptr) {
            glUniform2fv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::vec3>(name, size) };
        if (val != nullptr) {
            glUniform3fv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::vec4>(name, size) };
        if (val != nullptr) {
            glUniform4fv(location, static_cast<GLsizei>(size), glm::value_ptr(*val));
        }
    },
    /**************************************** Mat2xN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat2x2>(name, size) };
        if (val != nullptr) {
            glUniformMatrix2fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat2x3>(name, size) };
        if (val != nullptr) {
            glUniformMatrix2x3fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat2x4>(name, size) };
        if (val != nullptr) {
            glUniformMatrix2x4fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    /**************************************** Mat3xN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat3x2>(name, size) };
        if (val != nullptr) {
            glUniformMatrix3x2fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat3x3>(name, size) };
        if (val != nullptr) {
            glUniformMatrix3fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat3x4>(name, size) };
        if (val != nullptr) {
            glUniformMatrix3x4fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    /**************************************** Mat4xN ****************************************/
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat4x2>(name, size) };
        if (val != nullptr) {
            glUniformMatrix4x2fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat4x3>(name, size) };
        if (val != nullptr) {
            glUniformMatrix4x3fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    [](const ShaderEnvironment& environment, GLuint location, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<glm::mat4x4>(name, size) };
        if (val != nullptr) {
            glUniformMatrix4fv(location, static_cast<GLsizei>(size), false, glm::value_ptr(*val));
        }
    },
    /**************************************** SamplerN ****************************************/
    [](const ShaderEnvironment& environment, GLuint, std::string_view name, std::size_t size) {
        auto val{ environment.getPtr<TextureSampler<Texture2D>>(name, size) };
        if (val != nullptr) {
            val->bind();
        }
    },
};

ShaderProgram::ShaderProgram()
    : m_bound{ false }
    , m_program{ glCreateProgram() }
    , m_parameters{}
    , m_parameterLocations{}
{
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : m_bound{ std::exchange(other.m_bound, false) }
    , m_program{ std::exchange(other.m_program, 0) }
    , m_parameters{ std::exchange(other.m_parameters, {}) }
    , m_parameterLocations{ std::exchange(other.m_parameterLocations, {}) }
{
}

ShaderProgram::~ShaderProgram()
{
    unbind();
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    if (this != &other) {
        unbind();
        if (m_program != 0) {
            glDeleteProgram(m_program);
        }

        m_bound = std::exchange(other.m_bound, false);
        m_program = std::exchange(other.m_program, 0);
        m_parameters = std::exchange(other.m_parameters, {});
        m_parameterLocations = std::exchange(other.m_parameterLocations, {});
    }
    return *this;
}

void ShaderProgram::bind()
{
    if (!m_bound) {
        glUseProgram(m_program);
        m_bound = true;
    }
}
void ShaderProgram::unbind()
{
    if (m_bound) {
        glUseProgram(0);
        m_bound = false;
    }
}

void ShaderProgram::apply(const ShaderEnvironment& environment) const
{
    if (!m_bound) {
        return;
    }

    for (auto parameter : environment.parameters()) {
        auto declarationPos{ std::find_if(m_parameters.begin(), m_parameters.end(),
            [&](const ParameterDeclaration& decl) -> bool { return std::get<3>(decl) == parameter; }) };

        if (declarationPos == m_parameters.end()) {
            continue;
        }

        auto& declaration{ *declarationPos };
        auto parameterType{ std::get<1>(declaration) };
        auto parameterSize{ static_cast<GLsizei>(std::get<2>(declaration)) };
        auto parameterLocation{ m_parameterLocations.at(parameter) };

        if (static_cast<std::size_t>(parameterType) <= static_cast<std::size_t>(ParameterType::MaxIndex)) {
            auto applyFunc{ sTypeApplyFuncs[static_cast<std::size_t>(parameterType)] };
            applyFunc(environment, parameterLocation, parameter, parameterSize);
        }
    }
}

std::span<const ParameterDeclaration> ShaderProgram::parameters() const
{
    return { m_parameters.data(), m_parameters.size() };
}

}