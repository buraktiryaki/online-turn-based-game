#version 330 core
out vec4 FragColor;

struct PointLight {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform PointLight camLight;
uniform sampler2D texture_diffuse1;

//team color
uniform vec4 color;

void main()
{    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(camLight.position - FragPos);

    //calc. diffuse coef
    float diff = max(dot(norm, lightDir), 0.0);
    //calc. specular coef
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f); 

    //no attenuation: like global light

    //combine result
    vec3 tCol = vec3(texture(texture_diffuse1, TexCoords)) * color.xyz;
    //vec3 tCol = vec3(texture(texture_diffuse1, TexCoords));
    vec3 ambient = camLight.ambient * tCol;
    vec3 diffuse = camLight.diffuse * diff * tCol;
    vec3 specular = camLight.specular * spec;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0f);
}



