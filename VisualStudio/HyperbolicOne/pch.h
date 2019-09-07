

#pragma once


#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>  // will include d2d1_1.h for D2D1_BITMAP_PROPERTIES1 structure;
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>



#include <agile.h>  // ghv : if want to #include <agile.h> then must compile with /ZW.
#include <concrt.h>









#include <complex>
#include <vector>    //  ghv ;
#include <map>       //  ghv : 2018_07_29
#include <d2d1helper.h>   //  ghv : 2018_08_03 : for ctor D2D1::Point2F(); 
#include <random>
#include <assert.h>


#include "Keyboard.h"
#include "Mouse.h"
#include "WICTextureLoader.h"   //  CWS  --->  DirectXTK;

#pragma comment(lib, "DirectXTK")



#define GHV_OPTION_ENABLE_HUD




namespace HvyDXBase
{
    extern double const konst_pi;

    using HvyPlex = std::complex<double>;
}







