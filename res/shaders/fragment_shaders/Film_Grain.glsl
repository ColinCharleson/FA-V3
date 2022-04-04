void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = fragCoord/iResolution.xy;

    // Calculate noise and sample texture
    float mdf = 0.6; // increase for noise amount 
    float noise = (fract(sin(dot(uv, vec2(12.9898,78.233)*2.0)) * 43758.5453));
    vec4 tex = texture(iChannel0, uv);
    
    mdf *= sin(iTime) + 1.0; // animate the effect's strength
    
    vec4 col = tex - noise * mdf;

    // Output to screen
    fragColor = col;
}