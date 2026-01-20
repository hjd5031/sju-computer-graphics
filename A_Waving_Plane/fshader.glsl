#version 330



in vec4 color;
in vec4 position;   

out vec4 fColor;

void main()
{
    
    vec4 ColorUp   = vec4(1.0, 0.44, 0.0, 1.0);   //Orange
    vec4 ColorDown = vec4(0.11, 0.56, 1, 1.0);   //Blue

    
    float alpha = abs(position.z) / 0.4;//alpha proportional to height

    vec4 newColor;
    if (position.z > 0.0)
        newColor = ColorUp*alpha + color*(1-alpha);   // z > 0 -> Orange
    else
        newColor = ColorDown*alpha + color*(1-alpha); // z <= 0 -> blue

  
    fColor = newColor;
}