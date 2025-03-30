#version 330 core

// Data from vertex shader.
// --------------------------------------------------------
// Add your data for interpolation.
// --------------------------------------------------------
in vec3 iPosWorld;
in vec3 iNormalWorld;
in vec2 iTexCoord;
// --------------------------------------------------------
// Add your uniform variables.
// --------------------------------------------------------
// Transformation matrix.
uniform mat4 viewMatrix;
// Camera position.
uniform vec3 cameraPos;
// Material properties.
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
uniform sampler2D mapKd;
uniform bool hasMapKd;
// Light data.
uniform vec3 ambientLight;
uniform vec3 dirLightDir;
uniform vec3 dirLightRadiance;
uniform vec3 pointLightPos;
uniform vec3 pointLightIntensity;
uniform vec3 spotLightPos;
uniform vec3 spotLightIntensity;
uniform vec3 spotLightDir;
uniform float spotLightCutoffDeg;
uniform float spotLightTotalWidthDeg;

out vec4 FragColor;

vec3 Diffuse(vec3 texColor, vec3 I, vec3 N, vec3 L)
{
    return texColor * I * max(0.0, dot(N, L));
}

vec3 Specular(vec3 Ks, vec3 I, vec3 L, vec3 N, vec3 E, float ShininessStrength)
{
    // Try to implement yourself!

    // Phong
    //vec3 R = normalize(2 * dot(N, L) * N - L);
    //vec3 R = normalize(reflect(-L, N));
    //return Ks * I * pow(max(0, dot(E, R)), ShininessStrength);

    // Blinn-Phong
    vec3 H = normalize(L + E);
    return Ks * I * pow(max(0, dot(N, H)), ShininessStrength);
}

void main()
{
    // --------------------------------------------------------
    // Add your implementation.
    // --------------------------------------------------------
    vec3 N = normalize(iNormalWorld);
    // view dir.
    vec4 tmpPos = viewMatrix * vec4(cameraPos, 1.0);
    vec3 vCameraPos = tmpPos.xyz / tmpPos.w;
    vec3 E = normalize(vCameraPos - iPosWorld);
    // Texture color.
    // if hasMapKd => texColor = texture2D(mapKd, iTexCoord).rgb.
    // else texColor = Kd.
    vec3 texColor;
    if (hasMapKd)
         texColor = texture2D(mapKd, iTexCoord).rgb;
    else
        texColor = Kd;
    // -------------------------------------------------------------
    // Ambient light.
    vec3 ambient = Ka * ambientLight;
    // -------------------------------------------------------------
    // Compute fragment linghting in "view space"
    // v: view space
    // -------------------------------------------------------------
    // Directional light.
    vec3 vDirLightdir = (viewMatrix * vec4(-dirLightDir, 0.0)).xyz;
    vDirLightdir = normalize(vDirLightdir);
    // Diffuse.
    vec3 diffuse = Diffuse(texColor, dirLightRadiance, N, vDirLightdir);
    // Specular.
    vec3 specular = Specular(Ks, dirLightRadiance, vDirLightdir, N, E, Ns);
    vec3 dirLight = diffuse + specular;
    // -------------------------------------------------------------
    // Point light.
    tmpPos = viewMatrix * vec4(pointLightPos, 1.0);
    vec3 vPointLightPos = tmpPos.xyz / tmpPos.w;
    vec3 vPointLightDir = normalize(vPointLightPos - iPosWorld);
    float distSurfaceToPointLight = distance(vPointLightPos, iPosWorld);
    float PointLightAttenuation = 1.0f / (distSurfaceToPointLight * distSurfaceToPointLight);
    vec3 PointLightRadiance = pointLightIntensity * PointLightAttenuation;
    // Diffuse.
    diffuse = Diffuse(texColor, PointLightRadiance, N, vPointLightDir);
    // Specular.
    specular = Specular(Ks, PointLightRadiance, vPointLightDir, N, E, Ns);
    vec3 pointLight = diffuse + specular;
    // -------------------------------------------------------------
    // Spot light.
    tmpPos = viewMatrix * vec4(spotLightPos, 1.0);
    vec3 vSpotLightPos = tmpPos.xyz / tmpPos.w;
    vec3 vSpotLightToPos = normalize(vSpotLightPos - iPosWorld);
    vec3 vSpotLightdir = (viewMatrix * vec4(-spotLightDir, 0.0)).xyz;
    vSpotLightdir = normalize(vSpotLightdir);
    float distSurfaceToSpotLight = distance(vSpotLightPos, iPosWorld);
    float A = degrees(acos(dot(vSpotLightToPos, vSpotLightdir)));
    // (cosA - cosT) / (cosF - cosT).
    float SpotLightAttenuation = clamp((A - spotLightTotalWidthDeg) / (spotLightCutoffDeg - spotLightTotalWidthDeg), 0, 1);
    SpotLightAttenuation /= (distSurfaceToSpotLight * distSurfaceToSpotLight);
    vec3 SpotLightRadiance = spotLightIntensity * SpotLightAttenuation;
    // Diffuse.
    diffuse = Diffuse(texColor, SpotLightRadiance, N, vSpotLightToPos);
    // Specular.
    specular = Specular(Ks, SpotLightRadiance, vSpotLightToPos, N, E, Ns);
    vec3 spotLight = diffuse + specular;

    vec3 LightColor = ambient + dirLight + pointLight + spotLight;
    FragColor = vec4(LightColor, 1.0);
}