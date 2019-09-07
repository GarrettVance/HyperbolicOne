//      
//      


#define GHV_OPTION_USE_SAMPLER

#undef GHV_OPTION_AMAZING_WEIRD

#undef GHV_OPTION_COLOR_INVERT



Texture2D           ColorTexture        :       register(t0);
SamplerState        LinearSampler       :       register(s0);



cbuffer conbuf7 : register(b1)
{
    int         schlafli_p;
    int         schlafli_q;
    float       apothem;
    float       pixWidth;
}







struct PixelShaderInput
{
	float4      s_pos       : SV_POSITION;
	float3      s_color     : COLOR0;
    float2      s_texco     : TEXCOORD0;
};



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


static const  double konst_pi = 3.1415926535897932384626433;


static const int maxIterations = 128;


static const     float4      color0 = float4(0.0, 0.0, 1.0, 1.0);  // Interstitial color;
static const     float4      color1 = float4(0.0, 0.0, 0.0, 1.0);  // Color for the "negative" Fundamental Domain;
static const     float4      color2 = float4(1.0, 1.0, 1.0, 1.0);  // Color for "positive" Fundamental domain;













float4 invertColorRGBA(float4 c0)
{
#ifdef GHV_OPTION_COLOR_INVERT
    float4 antiColor = float4(
        1.0 - c0.x,
        1.0 - c0.y,
        1.0 - c0.z,
        c0.w  //  Leave c0.w  alpha channel unchanged;
        );
    return antiColor;
#else
    return c0;
#endif
}








float2 std_conj(float2 a)
{
    return float2(
        a.x,
        -a.y
    );
}







//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



float squared_norm(float2 a)
{ 
    //  the "squared" norm: 
    //  ===================
    //  or could use MSFT intrinsic vector dot product... TODO: 

    return a.x * a.x + a.y * a.y;
}







float std_abs(float2 a)
{
    return sqrt(squared_norm(a)); 
}





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



float std_arg(float2 a)
{
    //   ghv : better to use atan2() rather than plain old atan()

    return atan2(a.y, a.x); 
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

float2 mul_complex(float2 a, float2 b) 
{
    return float2(
        a.x * b.x - a.y * b.y, 
        a.x * b.y + a.y * b.x
    );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

float2 mulbis_complex(float2 a, float2 b) 
{ 
    //  return the product  a times (b bar)  

    return float2(
         a.x * b.x + a.y * b.y, 
        -a.x * b.y + a.y * b.x
    );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

float2 div_complex(float2 a, float2 b) 
{ 
    //   return the quotient  a / b

    float d = squared_norm(b);

    return float2(
        (a.x * b.x + a.y * b.y) / d, 
        (-a.x * b.y + a.y * b.x) / d
    );
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



float2 hyper_translate(float2 p_amount, float2 p_z) 
{ 
    return div_complex(
        p_z + p_amount, 
        float2(1.0, 0.0) + mulbis_complex(p_z, p_amount)
    );
}




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++







//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

float2 std_polar(float a)
{
    return float2(
        cos(a), 
        sin(a)
    );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



float2 isometryA(float2 z)
{
    //  Reflect across the edge opposite vertex 0 at origin;
    //  ====================================================

    float2 u = hyper_translate(
        float2(-apothem, 0.0), 
        z
    );

    //  Reflect d0 across the imaginary axis: 
    //  (Note the signum).

    float2 v = -std_conj(u);

    float2 reflected = hyper_translate(
        float2(apothem, 0.0), 
        v
    );

    return reflected;
}






bool insideFDTNarrow(float2 z)
{
    bool insideSector = (
        (std_arg(z) < konst_pi / schlafli_p) &&
        (std_arg(z) > 0.00));

    float2 w = hyper_translate(
        float2(-apothem, 0.0), 
        z
    );

    bool insideGeodesicArc = (w.x < 0.00);

    return insideSector && insideGeodesicArc;
}






float2 FloorSector(float2 z_input)
{

#ifdef GHV_OPTION_AMAZING_WEIRD
    int p_or_q = schlafli_q; // Weird accident of nature;
#else
    int p_or_q = schlafli_p; // Actual mathematics;
#endif


    float2 z_return = z_input;

    float arg_radians = std_arg(z_input);

    float s = arg_radians * p_or_q / (float)(2 * konst_pi);

    //  Rotate z_input until it resides inside
    //  the sector -pi/p < theta < pi/p.
    //  Visualize this sector as straddling the positive real axis
    //  and having TOTAL included angle 2pi/p. 

    if (std_abs(s) > 0.5)
    {
        int n = (int)floor(0.5 + s);

        float angle1 = -n * 2 * (float)konst_pi / p_or_q;

        z_return = mul_complex(
            z_input,
            std_polar(angle1)
            );
    }

    return z_return;
}






float4 ps_main(PixelShaderInput input) : SV_TARGET
{
    float4 retColor = float4(1.0, 1.0, 1.0, 1.0);

    if(squared_norm(input.s_texco) < 0.98) 
    {
        float2 cxApothem = float2(apothem, 0.0); 

        float2 zfs = input.s_texco; 

        uint spin = 0;

        bool amInside = false;

        float2 zit = zfs;  //  Don't allow modification of zfs; Use zit as its updatable proxy;

        float2 zinvrtd = float2(0.0, 0.0); 

        float2 zinsideFDT = float2(0.0, 0.0); 

        float theta = std_arg(zfs);


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        

        for (int idxIteration = 0; idxIteration < maxIterations; idxIteration++)
        {
            float2 rotz = FloorSector(zit); // Apply first isometry, namely rotation;

            if (rotz.y < 0.00)
            {
                spin++;
                rotz = std_conj(rotz);
            }

            //  Test whether or not inside: 

            if (insideFDTNarrow(rotz))
            {
                amInside = true;
                zinsideFDT = rotz;
                break;
            }

            //  Apply the second isometry, namely reflection (inversion in circle) "A()": 

            zinvrtd = isometryA(rotz);

            spin++;  // Increment Spin by 1 because the isometry applied just now was a reflection; 

            //  Test whether or not inside: 

            if (insideFDTNarrow(zinvrtd))
            {
                amInside = true;
                zinsideFDT = zinvrtd;
                break;
            }

            //  Iterate as needed to get the given point 
            //  inside the fundamental domain triangle:

            zit = zinvrtd;
        }
        //  Closes the "for" loop; 


        float2 w = zinsideFDT; 

        float2 gawgai = float2(0.5 + w.x, 0.5 - w.y);

        if (w.y < 0.00) { gawgai = float2(0.5 + w.x, 0.5 + w.y); }

  
        //  Draw the point. Color is determined by the parity of "spin": 

        float edgeThickness = 0.0002; // formerly 0.02;

        if (amInside)
        {
            float2 edge = mul_complex(
                std_polar((float)(-konst_pi / 2.00)),   
                hyper_translate(
                    float2(-apothem, 0.0), 
                    zinsideFDT)
                );

            if (edge.y * edge.y < edgeThickness)
            {
                retColor = color0;
            }
            else if (spin % 2 == 0)
            {
#ifdef GHV_OPTION_USE_SAMPLER
                retColor = ColorTexture.Sample(LinearSampler, gawgai);
#else
                retColor = color1;
#endif
            }
            else
            {
#ifdef GHV_OPTION_COLOR_INVERT
                retColor = invertColorRGBA(color2);
#else
                retColor = color2;
#endif

#ifdef GHV_OPTION_USE_SAMPLER
                retColor = invertColorRGBA(ColorTexture.Sample(LinearSampler, gawgai));
#endif
            }
        }
    }
    //  Closes "if squared_norm..."; 

    else if(squared_norm(input.s_texco) < 1.00)
    {
        retColor = float4(0.0, 0.0, 0.0, 1.0);  //  paint it black;
    }
    else
    {
        retColor = float4(0.0, 0.0, 0.0, 0.0);  // paint is clear black;?;
    }

    return retColor;
}









