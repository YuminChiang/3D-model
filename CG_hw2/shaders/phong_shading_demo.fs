#version 330 core

// Data from vertex shader.
// --------------------------------------------------------
// Add your data for interpolation.
// --------------------------------------------------------
in vec3 iPosWorld;
in vec3 iNormalWorld;
// --------------------------------------------------------
// Add your uniform variables.
// --------------------------------------------------------
// Camera position.
uniform vec3 cameraPos;
// Material properties.
uniform vec3 Ka;
uniform vec3 Kd;
uniform vec3 Ks;
uniform float Ns;
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

vec3 Diffuse(vec3 Kd, vec3 I, vec3 N, vec3 L)
{
    return Kd * I * max(0, dot(N, L));
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
    vec3 E = normalize(cameraPos - iPosWorld);
    // -------------------------------------------------------------
    // Ambient light.
    vec3 ambient = Ka * ambientLight;
    // -------------------------------------------------------------
    // Compute fragment linghting in "world space"
    // w: world space
    // -------------------------------------------------------------
    // Directional light.
    vec3 wDirLightDir = normalize(-dirLightDir);
    // Diffuse.
    vec3 diffuse = Diffuse(Kd, dirLightRadiance, N, wDirLightDir);
    // Specular.
    vec3 specular = Specular(Ks, dirLightRadiance, wDirLightDir, N, E, Ns);
    vec3 dirLight = diffuse + specular;
    // -------------------------------------------------------------
    // Point light.
    vec3 wPointLightDir = normalize(pointLightPos - iPosWorld);
    float distSurfaceToPointLight = distance(pointLightPos, iPosWorld);
    float PointLightAttenuation = 1.0f / (distSurfaceToPointLight * distSurfaceToPointLight);
    vec3 PointLightRadiance = pointLightIntensity * PointLightAttenuation;
    // Diffuse.
    diffuse = Diffuse(Kd, PointLightRadiance, N, wPointLightDir);
    // Specular.
    specular = Specular(Ks, PointLightRadiance, wPointLightDir, N, E, Ns);
    vec3 pointLight = diffuse + specular;
    // -------------------------------------------------------------
    // Spot light.
    vec3 wSpotLightToPos = normalize(spotLightPos - iPosWorld);
    vec3 wSpotLightDir = normalize(spotLightDir);
    float distSurfaceToSpotLight = distance(spotLightPos, iPosWorld);
    float A = degrees(acos(dot(wSpotLightToPos, -wSpotLightDir)));
    // (cosA - cosT) / (cosF - cosT).
    float SpotLightAttenuation = clamp((A - spotLightTotalWidthDeg) / (spotLightCutoffDeg - spotLightTotalWidthDeg), 0, 1);
    SpotLightAttenuation /= (distSurfaceToSpotLight * distSurfaceToSpotLight);
    vec3 SpotLightRadiance = spotLightIntensity * SpotLightAttenuation;
    // Diffuse.
    diffuse = Diffuse(Kd, SpotLightRadiance, N, wSpotLightToPos);
    // Specular.
    specular = Specular(Ks, SpotLightRadiance, wSpotLightToPos, N, E, Ns);
    vec3 spotLight = diffuse + specular;
    
    FragColor = vec4((ambient + dirLight + pointLight + spotLight), 10);
}