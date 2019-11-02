
#include "pch.h"

#include "Hvy3DScene.h"
#include "..\Common\DirectXHelper.h"

using namespace HyperbolicOne;
using namespace DirectX;
using namespace Windows::Foundation;



void Hvy3DScene::Initialize_Schlafli(int schlafli_p, int schlafli_q)
{
    e_schlafli_p = schlafli_p;
    e_schlafli_q = schlafli_q;

    e_circumradius = HvyDXBase::HC_CircumradiusFromSchlafli_D(schlafli_p, schlafli_q);

    //  Compute the apothem

    e_apothem = HvyDXBase::HC_ApothemFromSchlafli_D(schlafli_p, schlafli_q); 
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

Hvy3DScene::Hvy3DScene(const std::shared_ptr<DX::DeviceResources>& deviceResources) 
    :
    m_loadingComplete(false),
    m_degreesPerSecond(45),
    m_indexCount(0),
    m_deviceResources(deviceResources)
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

    Initialize_Schlafli(5, 6);  // Classic is Schlafli {7,3}; 
    // Initialize_Schlafli(7, 3);  // Classic is Schlafli {7,3}; 
}


void Hvy3DScene::CreateWindowSizeDependentResources()
{
    Size outputSize = m_deviceResources->GetOutputSize();
    float aspectRatio = outputSize.Width / outputSize.Height;
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // Note that the OrientationTransform3D matrix is post-multiplied here
    // in order to correctly orient the scene to match the display orientation.
    // This post-multiplication step is required for any draw calls that are
    // made to the swap chain render target. For draw calls to other targets,
    // this transform should not be applied.


    // Hvy3DScene makes use of a right-handed coordinate system using row-major matrices.

    XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
        fovAngleY,
        aspectRatio,
        0.01f,
        100.0f
        );

    XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

    XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

    XMStoreFloat4x4(
        &m_conbuf_MVP_Data.projection,
        XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
        );

    XMStoreFloat4x4(
        &m_conbuf_MVP_Data.view, 
        XMMatrixIdentity()
    );
}


void Hvy3DScene::Update(DX::StepTimer const& timer)
{
    float uniFactor = 0.7f; 

    DirectX::XMStoreFloat4x4(
        &m_conbuf_MVP_Data.model,
        XMMatrixTranspose(
            XMMatrixScaling(uniFactor, uniFactor, 1.0f)
        )
    );
}


void Hvy3DScene::Render()
{
    if (!m_loadingComplete)  // Loading is asynchronous. Only draw geometry after it's loaded.
    {
        return;
    }

    conbufSetDataHC();

    auto d3d_context_3d = m_deviceResources->GetD3DDeviceContext();

    d3d_context_3d->UpdateSubresource1( m_conbuf_MVP_Buffer.Get(), 0, NULL, &m_conbuf_MVP_Data, 0, 0, 0 );

    UINT stride = sizeof(Vertex_Pos_Tex_Normal_t);
    UINT offset = 0;
    d3d_context_3d->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

    //  Index Buffer: Each index is one 16-bit unsigned integer (short): 
    d3d_context_3d->IASetIndexBuffer( m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0 );

    d3d_context_3d->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    d3d_context_3d->IASetInputLayout(m_inputLayout.Get());

    d3d_context_3d->VSSetShader( m_vertexShader.Get(), nullptr, 0 );
    d3d_context_3d->VSSetConstantBuffers1( 0, 1, m_conbuf_MVP_Buffer.GetAddressOf(), nullptr, nullptr );

    d3d_context_3d->RSSetState(e_rasterizer_state.Get());

    d3d_context_3d->PSSetShader(m_pixelShader.Get(), nullptr, 0 );
    d3d_context_3d->PSSetConstantBuffers1( 1, 1, m_conbuf_HC_Buffer.GetAddressOf(), nullptr, nullptr ); // Slot 1;
    d3d_context_3d->PSSetShaderResources(0, 1, e_srv_FunDomain.GetAddressOf());
    d3d_context_3d->PSSetSamplers(0, 1, e_SamplerState_for_FunDomain.GetAddressOf());

    d3d_context_3d->DrawIndexed( m_indexCount, 0, 0 );
}


void Hvy3DScene::conbufSetDataHC()
{
    conbuf7Struct tmp7Struct = { 
        e_schlafli_p, 
        e_schlafli_q, 
        (float)e_apothem,
        (float)1.f  // This field is just to pad the structure to 16 bytes wide; 
    }; 

    auto d3d_context_3d = m_deviceResources->GetD3DDeviceContext();

    D3D11_MAPPED_SUBRESOURCE mapped_subresource_cb7;
    ZeroMemory(&mapped_subresource_cb7, sizeof(D3D11_MAPPED_SUBRESOURCE));
    d3d_context_3d->Map(m_conbuf_HC_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource_cb7);
    memcpy(
        mapped_subresource_cb7.pData,
        &tmp7Struct, 
        sizeof(conbuf7Struct)
    );
    d3d_context_3d->Unmap(m_conbuf_HC_Buffer.Get(), 0);
}


void Hvy3DScene::CreateDeviceDependentResources()
{
    D3D11_RASTERIZER_DESC   rasterizer_description;
    ZeroMemory(&rasterizer_description, sizeof(rasterizer_description));
    rasterizer_description.MultisampleEnable = FALSE;
    rasterizer_description.FillMode = D3D11_FILL_SOLID;
    rasterizer_description.FrontCounterClockwise = false;
    rasterizer_description.CullMode = D3D11_CULL_BACK; 

    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(
        &rasterizer_description,
        e_rasterizer_state.ReleaseAndGetAddressOf()
    ));

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    //  Sampler State for the Pixel Shader to render the 
    //  Fundamental Domain Triangle and its reflections: 
    {
        D3D11_SAMPLER_DESC sampDescFunDomain;
        ZeroMemory(&sampDescFunDomain, sizeof(sampDescFunDomain));
        sampDescFunDomain.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

        sampDescFunDomain.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDescFunDomain.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDescFunDomain.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

        sampDescFunDomain.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDescFunDomain.MinLOD = 0;
        sampDescFunDomain.MaxLOD = D3D11_FLOAT32_MAX;

        DX::ThrowIfFailed(
            m_deviceResources->GetD3DDevice()->CreateSamplerState(
                &sampDescFunDomain,
                e_SamplerState_for_FunDomain.ReleaseAndGetAddressOf()
            )
        );
    }

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    Microsoft::WRL::ComPtr<ID3D11Resource>  temp_resource;

    wchar_t file_path_to_image[] = L"Assets\\a_test.png";

    DX::ThrowIfFailed(
        CreateWICTextureFromFile(
            m_deviceResources->GetD3DDevice(), 
            file_path_to_image,
            temp_resource.ReleaseAndGetAddressOf(),
            e_srv_FunDomain.ReleaseAndGetAddressOf(),
            0)
    );

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    D3D11_RENDER_TARGET_BLEND_DESC  rt_blend_descr = { 0 };
    rt_blend_descr.BlendEnable = TRUE;

    rt_blend_descr.SrcBlend = D3D11_BLEND_SRC_ALPHA;        // SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rt_blend_descr.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;   // DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rt_blend_descr.BlendOp = D3D11_BLEND_OP_ADD;            // BlendOp = D3D11_BLEND_OP_ADD;

    rt_blend_descr.BlendOp = D3D11_BLEND_OP_SUBTRACT; // undo

    rt_blend_descr.SrcBlendAlpha = D3D11_BLEND_ONE;
    rt_blend_descr.DestBlendAlpha = D3D11_BLEND_ZERO; 

    rt_blend_descr.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt_blend_descr.BlendOpAlpha = D3D11_BLEND_OP_SUBTRACT; // undo

    rt_blend_descr.RenderTargetWriteMask = 0x0F;

#undef GHV_OPTION_ENABLE_D3D11_BLEND_WEIRD
#ifdef GHV_OPTION_ENABLE_D3D11_BLEND_WEIRD
    //   Weird effect: the d2d1 lines become glassy transparent: 
    rt_blend_descr.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rt_blend_descr.DestBlend = D3D11_BLEND_INV_SRC_COLOR;
    rt_blend_descr.BlendOp = D3D11_BLEND_OP_ADD;
    rt_blend_descr.SrcBlendAlpha = D3D11_BLEND_ONE;
    rt_blend_descr.DestBlendAlpha = D3D11_BLEND_ZERO;
    rt_blend_descr.BlendOpAlpha = D3D11_BLEND_OP_ADD;
#endif


    D3D11_BLEND_DESC  d3d11_blend_descr = { 0 };
    d3d11_blend_descr.AlphaToCoverageEnable = TRUE; // undo
    d3d11_blend_descr.IndependentBlendEnable = TRUE;
    d3d11_blend_descr.RenderTarget[0] = { rt_blend_descr };

    DX::ThrowIfFailed(
        m_deviceResources->GetD3DDevice()->CreateBlendState(
            &d3d11_blend_descr, 
            s3d_blend_state.GetAddressOf()
        )
    );

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    {
        D3D11_BUFFER_DESC conbufDescHCBuffer;
        ZeroMemory(&conbufDescHCBuffer, sizeof(D3D11_BUFFER_DESC));
        conbufDescHCBuffer.Usage = D3D11_USAGE_DYNAMIC;
        conbufDescHCBuffer.ByteWidth = sizeof(conbuf7Struct);
        conbufDescHCBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        conbufDescHCBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        conbufDescHCBuffer.MiscFlags = 0;

        static_assert((sizeof(conbuf7Struct) % 16) == 0, "Constant buffer struct must be 16-byte aligned");

        DX::ThrowIfFailed(
            m_deviceResources->GetD3DDevice()->CreateBuffer(
                &conbufDescHCBuffer,
                nullptr,
                &m_conbuf_HC_Buffer
            )
        );
    }

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    // Load shaders asynchronously.

    auto loadVS_2D_Task = DX::ReadDataAsync(L"t1VertexShader.cso");
    auto loadPS_01_Task = DX::ReadDataAsync(L"t1PixelShader.cso");

    auto createVS_2D_Task = loadVS_2D_Task.then([this](const std::vector<byte>& fileData) 
    {
        DX::ThrowIfFailed(
            m_deviceResources->GetD3DDevice()->CreateVertexShader( 
                &fileData[0], 
                fileData.size(), 
                nullptr, 
                &m_vertexShader
            ) 
        );

        static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
        {
            { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0,                             D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,    0,  D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        DX::ThrowIfFailed( 
            m_deviceResources->GetD3DDevice()->CreateInputLayout( 
                vertexDesc, 
                ARRAYSIZE(vertexDesc), 
                &fileData[0], 
                fileData.size(), 
                &m_inputLayout 
            ) 
        );
    });


    auto createPS_01_Task = loadPS_01_Task.then([this](const std::vector<byte>& fileData) {

        DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreatePixelShader(
                &fileData[0], fileData.size(), nullptr, &m_pixelShader)
        );

        CD3D11_BUFFER_DESC constantBufferDesc(sizeof(conbufMVPStruct) , D3D11_BIND_CONSTANT_BUFFER);

        static_assert((sizeof(conbufMVPStruct) % 16) == 0, "Constant buffer struct must be 16-byte aligned");

        DX::ThrowIfFailed( m_deviceResources->GetD3DDevice()->CreateBuffer( &constantBufferDesc, nullptr, &m_conbuf_MVP_Buffer ) );
    });


    auto createCubeTask = (createPS_01_Task && createVS_2D_Task).then([this]()
    {
        this->MeshMonoQuad();  // ghv : one quad composed of two triangles;
    });


    createCubeTask.then([this] () {
        m_loadingComplete = true;
    });
}


void Hvy3DScene::ReleaseDeviceDependentResources()
{
    m_loadingComplete = false;
    m_vertexShader.Reset();
    m_inputLayout.Reset();
    m_pixelShader.Reset();
    m_conbuf_MVP_Buffer.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
}


void Hvy3DScene::MeshMonoQuad()
{
    // ghv : Values of texco texture coordinates will be 
    // set inside the VS vertex shader. 

    float a = 0.577f;  //  sqrt of one-third; 

    float u_min = 0.0f; float u_max = 1.f;
    float v_min = 0.0f; float v_max = 1.f;

    static const Vertex_Pos_Tex_Normal_t   monoQuadVertices[] =
    {
        //   increasing negative values of z push the mesh deeper into the background: 

        {XMFLOAT3(-1.0f, -1.0f,  -1.f), XMFLOAT2(u_min, v_max), XMFLOAT3(0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(+1.0f, -1.0f,  -1.f), XMFLOAT2(u_max, v_max), XMFLOAT3(0.0f, 0.0f, 0.0f)},
        {XMFLOAT3(-1.0f, +1.0f,  -1.f), XMFLOAT2(u_min, v_min), XMFLOAT3(0.0f, 0.0f, 0.0f)},

        {XMFLOAT3(-1.0f, +1.0f,  -1.f), XMFLOAT2(u_min, v_min), XMFLOAT3(1.0f, 1.0f, 1.0f)},
        {XMFLOAT3(+1.0f, -1.0f,  -1.f), XMFLOAT2(u_max, v_max), XMFLOAT3(1.0f, 1.0f, 1.0f)},
        {XMFLOAT3(+1.0f, +1.0f,  -1.f), XMFLOAT2(u_max, v_min), XMFLOAT3(1.0f, 1.0f, 1.0f)},
    };

    D3D11_SUBRESOURCE_DATA quad_vb_data = { 0 };
    quad_vb_data.pSysMem = monoQuadVertices;
    quad_vb_data.SysMemPitch = 0;
    quad_vb_data.SysMemSlicePitch = 0;

    CD3D11_BUFFER_DESC quad_vb_descr(sizeof(monoQuadVertices), D3D11_BIND_VERTEX_BUFFER);

    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(
        &quad_vb_descr, &quad_vb_data, &m_vertexBuffer
    )
    );

    //===============================================
    //  Each triangle below is FRONT_FACE_CLOCKWISE: 
    //          
    //  (cf D3D11_RASTERIZER_DESC.FrontCounterClockwise);
    //      
    //  Each trio of index entries represents one triangle. 
    //===============================================

    static const unsigned short quadIndices[] =
    {
        0,2,1,
        5,4,3,
    };

    m_indexCount = ARRAYSIZE(quadIndices);

    D3D11_SUBRESOURCE_DATA quad_ib_data = { 0 };
    quad_ib_data.pSysMem = quadIndices;
    quad_ib_data.SysMemPitch = 0;
    quad_ib_data.SysMemSlicePitch = 0;

    CD3D11_BUFFER_DESC quad_ib_descr(sizeof(quadIndices), D3D11_BIND_INDEX_BUFFER);

    DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer( &quad_ib_descr, &quad_ib_data, &m_indexBuffer ) );
}



