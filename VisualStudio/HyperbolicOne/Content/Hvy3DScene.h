


#pragma once

#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"
#include "..\HyperbolicMethods\HyperbolicMethods.h"


namespace HyperbolicOne
{



    struct conbufMVPStruct
    {
        DirectX::XMFLOAT4X4 model;
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
    };




    struct conbuf7Struct
    {
        int                     schlafli_p;
        int                     schlafli_q;
        float                   apothem;
        float                   uniform_oneOverScale;
    };


    //  okay:  size_t v = sizeof(conbuf7Struct);  // 16 Ui64;



    struct Vertex_Pos_Tex_Normal_t
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 tex;
        DirectX::XMFLOAT3 normal;
    };










	class Hvy3DScene
	{
	public:
		Hvy3DScene(const std::shared_ptr<DX::DeviceResources>& deviceResources);

		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();


		void Update(DX::StepTimer const& timer);
		void Render();


	private:

        void Initialize_Schlafli(int schlafli_p, int schlafli_q);


        void                        conbufSetDataHC();

        void                        MeshMonoQuad();


	private:

		std::shared_ptr<DX::DeviceResources> m_deviceResources;

	
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;





        Microsoft::WRL::ComPtr<ID3D11Buffer>                m_conbuf_MVP_Buffer;
        conbufMVPStruct	                                    m_conbuf_MVP_Data;




        Microsoft::WRL::ComPtr<ID3D11Buffer>                    m_conbuf_HC_Buffer;


        
        Microsoft::WRL::ComPtr<ID3D11RasterizerState>           e_rasterizer_state;
        Microsoft::WRL::ComPtr<ID3D11SamplerState>              e_SamplerState_for_FunDomain;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>        e_srv_FunDomain;
        Microsoft::WRL::ComPtr<ID3D11BlendState>                s3d_blend_state;

        
        
        uint32	m_indexCount;

	
		bool	m_loadingComplete;
		float	m_degreesPerSecond;



        int                                                     e_schlafli_p;
        int                                                     e_schlafli_q;
        double                                                  e_circumradius;
        double                                                  e_apothem;


        float                           uniform_oneOverScale;  // reciprocal of render target half-height; 
	};
}


