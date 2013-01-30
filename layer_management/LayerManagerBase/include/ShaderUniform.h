/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#ifndef _SHADERUNIFORM_H_
#define _SHADERUNIFORM_H_

#include <string>
using std::string;

#include <vector>

class ShaderProgram;

/**
 * Stores values for a shader uniform.
 */
class ShaderUniform
{
public:
    /** Uniform data value type */
    enum Type
    {
        Undefined,
        Vector1f,
        Vector2f,
        Vector3f,
        Vector4f,
        Matrix2f,
        Matrix3f,
        Matrix4f
    };

    /**
     * Create a uniform from a string description.
     *
     * The string description must in the format as follows:
     *    "name type count [transpose] value1 value2 ..."
     *
     * String parameter:
     *    name  = the uniform name
     *    type  = the element type (1f,2f,3f,4f,m2f,m3f,m4f)
     *    count = number of elements
     *    transpose = 0|1, only available if type is a matrix
     *    value = the data values
     *
     * Examples:
     *    "uBlurRadius 1f 1 2.7"
     *    "uSize 2f 1 2.5 1.25"
     *    "uTransform 4f 1 0 1.0 0.0 0.0 1.0 0.0 ..."
     *
     * @param description  The description
     * @return Uniform object or NULL in case of a parse error
     */
    static ShaderUniform* createFromStringDescription(const string& description);

    /**
     * constructor.
     *
     * @param name      Name of uniform
     * @param location  Uniform location in program
     */
    ShaderUniform(const string& name, int location = -1);

    /**
     * Copy constructor
     */
    ShaderUniform(const ShaderUniform& other);

    /**
     * Destructor
     */
    ~ShaderUniform();

    /**
     * @return Uniform name
     */
    const string& getName(void) const
    {
        return m_name;
    }

    /**
     * Set uniform data of type "float"
     * This can be Vector1f..Vector4f or Matrix2f..Matrix4f
     *
     * @param type   Data type
     * @param count  Number of data elements
     * @param values  Data values. Actual number of floats must be (count*(size of type)).
     * @param transpose  Whether to transpose a matrix. Only needed if element type is a matrix.
     */
    void setData(Type type, int count, const std::vector<float>& values, bool transpose = false);

    /**
     * Copy data from another uniform
     */
    void setData(const ShaderUniform& other);

    /**
     * Load uniform data in current OpenGL context.
     */
    void load(ShaderProgram& program);

private:
    /// the uniform name
    string m_name;

    /// location in shader program
    int m_location;

    /// the uniform data type
    Type m_type;

    /// number of data elements
    int m_count;

    /// the data for float types
    std::vector<float> m_floatData;

    /// whether to transpose the matrix array
    bool m_transpose;

    /// lookup table for element sizes
    static const int m_vectorSizesOfDataTypes[];
};

#endif /* _SHADERUNIFORM_H_ */

