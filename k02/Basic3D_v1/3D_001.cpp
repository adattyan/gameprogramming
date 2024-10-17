/*
	3D_001.cpp

	�͂��߂Ă�3D�iX�t�@�C���ǂݍ��݁j
*/
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <d3d9.h>			// Direct3D9 (DirectX9 Graphics)�ɕK�v
#include <d3dx9core.h>		// DirectX �X�v���C�g�ɕK�v
#include <tchar.h>
#include <assert.h>

// �E�B���h�E�̕��ƍ�����萔�Őݒ肷��
#define WIDTH 1024			//< ��
#define HEIGHT 768			//< ����

// �E�B���h�E�^�C�g���o�[�ɕ\�������o�[�W������
static const TCHAR version[] = _TEXT(__DATE__ " " __TIME__);

// �E�B���h�E�N���X��.
static const TCHAR wndClsName[] = _TEXT("directx");

// �N���X�錾�B���̂̒�`����ɃN���X��������錾���Ă����B
struct DataSet;
class MyXData;

// �֐��錾
HRESULT initDirect3D(DataSet* data);
const TCHAR* d3dErrStr(HRESULT res);
void ReleaseDataSet(DataSet* data);

// �v���O�����ɕK�v�ȕϐ����\���̂Ƃ��Ē�`
struct DataSet
{
	HINSTANCE hInstance;		//< �C���X�^���X�n���h��
	HWND hWnd;					//< �\���E�B���h�E
	IDirect3D9* pD3D;			//< Direct3D�C���X�^���X�I�u�W�F�N�g
	D3DPRESENT_PARAMETERS d3dpp;//< �f�o�C�X�쐬���̃p�����[�^
	IDirect3DDevice9* dev;		//< Direct3D�f�o�C�X�I�u�W�F�N�g

	// 3D�\���p�̃f�[�^
	float aspect;				//< ��ʃA�X�y�N�g��i�c����j
	float rotx, roty;			//< �I�u�W�F�N�g����]�������
	float transz;				//< �J������Z�ʒu

	// ���C�g��ON/OFF
	bool light[3];				//< ���C�g��ON/OFF�p�t���O

	MyXData* xdata;				//< X�t�@�C���̃f�[�^
} mydata;

////////////////////////// class MyXData //////////////////////////
// X�f�[�^���Ǘ�����N���X�B
// �����o�֐���load�����邪�A���̊֐���static�֐��B�܂�A�N���X�֐��B
// �C���X�^���X�𐶐����邽�߂̊֐��ƂȂ��Ă���B
class MyXData
{
private:
	// X�t�@�C���ǂݍ��݂̂��߂ɕK�v�ȕϐ�
	ID3DXMesh* pMesh_;				//< �`��f�[�^
	DWORD numMaterials_;			//< �}�e���A���f�[�^��
	IDirect3DTexture9** pTextures_;	//< �e�N�X�`�����
	D3DMATERIAL9* pMaterials_;		//< �}�e���A���f�[�^

public:
	MyXData() :pMesh_(0), numMaterials_(0), pTextures_(0), pMaterials_(0) {}
	~MyXData();
	void draw(DataSet* data);		//< �`��

	// static�����o�֐��́A�N���X�֐��ƂȂ�B
	static MyXData* load(DataSet* data, const TCHAR* fname);
};

// �f�X�g���N�^�ł͑S�f�[�^���������
MyXData::~MyXData()
{
	if (pMesh_) pMesh_->Release();
	for (unsigned int ii = 0; ii < numMaterials_; ii++)
	{
		if (pTextures_[ii]) pTextures_[ii]->Release();
	}
	if (pTextures_) delete[] pTextures_;
	if (pMaterials_) delete[] pMaterials_;
}

// X�t�@�C����ǂݍ��ށB�������g�𐶐����邽�߁Astatic�����o�֐��ƂȂ��Ă���B
// arg data �f�[�^�Z�b�g
// arg fname �t�@�C����
MyXData* MyXData::load(DataSet* data, const TCHAR* fname)
{
	MyXData* xxx = new MyXData();		// �C���X�^���X�𐶐�

	// �w�肷��X�t�@�C����ǂݍ���ŁA�V�X�e���������ɓW�J����B
	// �|���S�����b�V�����̑��ɁA�}�e���A�������ǂݍ��ށB
	HRESULT hr;
	ID3DXBuffer* pMB;		// �}�e���A�����̂��߂̃f�[�^�o�b�t�@

	// D3DXLoadMeshFromX�ɂ��X�t�@�C������|���S�����b�V����ǂݍ��ށB
	// �f�[�^�z�u�ꏊ�Ƃ���SYSTEMMEM(�V�X�e��������)��I������B
	// ����ɂ��t���X�N���[���؂�ւ����ɂ��f�[�^�͔j�󂳂�Ȃ��B
	hr = D3DXLoadMeshFromX(fname, D3DXMESH_SYSTEMMEM, data->dev, NULL,
						   &pMB, NULL, &xxx->numMaterials_, &xxx->pMesh_);
	if (hr != D3D_OK)
	{
		MessageBox(NULL, _TEXT("X�t�@�C���̓ǂݍ��݂Ɏ��s"), fname, MB_OK);
		return 0;
	}

	// �}�e���A�����ƃe�N�X�`�������擾���A���C���������ɕۑ����Ă����B
	// �}�e���A�������e�N�X�`�����Ƃ������肪�ł���̂ŁAnumMaterials_��
	// �������z����쐬����B
	xxx->pMaterials_ = new D3DMATERIAL9[xxx->numMaterials_];
	xxx->pTextures_ = new IDirect3DTexture9 * [xxx->numMaterials_];

	// �}�e���A�����ɃA�N�Z�X���邽�߁A�|�C���^�𓾂�B
	// �|�C���^�͔ėp�^�Ȃ̂ŁA�}�e���A���^�ɕϊ����Ȃ���΂Ȃ�Ȃ��B
	D3DXMATERIAL* mat = (D3DXMATERIAL*)pMB->GetBufferPointer();

	// �}�e���A���͕������肤��̂ŁA���̑S�Ăɂ��āA�����擾
	for (unsigned int ii = 0; ii < xxx->numMaterials_; ii++)
	{
		// MatD3D�̓}�e���A���i�F�j��񂪊i�[����Ă���\���́B
		xxx->pMaterials_[ii] = mat[ii].MatD3D;
		if (mat[ii].pTextureFilename) {
			// �e�N�X�`���f�[�^��ǂݍ��ށB
			TCHAR file_path[FILENAME_MAX] = { 0 };
#if defined(UNICODE)
			TCHAR tmp[FILENAME_MAX] = { 0 };
			int len = (int)strlen(mat[ii].pTextureFilename);
			assert(0 < len && len < FILENAME_MAX);
			if (0 < len && len < FILENAME_MAX) {
				MultiByteToWideChar(CP_ACP, MB_COMPOSITE, mat[ii].pTextureFilename, len, tmp, _countof(tmp));
			}
			_stprintf_s(file_path, _countof(file_path), _TEXT("data/%s"), tmp);
#else
			sprintf_s(file_path, "data/%s", mat[ii].pTextureFilename);
#endif
			hr = D3DXCreateTextureFromFile(data->dev, file_path, &(xxx->pTextures_[ii]));
			// �ǂݍ��ݎ��s�����ꍇ�̓e�N�X�`������
			if (hr != D3D_OK) {
				xxx->pTextures_[ii] = NULL;
			}
		}
		else {
			xxx->pTextures_[ii] = NULL;
		}
	}
	// �}�e���A���������
	pMB->Release();

	return xxx;
}

// X�f�[�^��`�悷��
void MyXData::draw(DataSet* data)
{
	for (unsigned int ii = 0; ii < this->numMaterials_; ii++)
	{
		// �e�N�X�`��������΁A�ݒ肷��
		data->dev->SetTexture(0, this->pTextures_[ii]);
		// �}�e���A������ݒ肵�A�|���S���ɐF������
		data->dev->SetMaterial(&(this->pMaterials_[ii]));

		// �|���S�����b�V����`��
		this->pMesh_->DrawSubset(ii);
	}
}

////////////////////////// ::setCamera //////////////////////////
// �J������ݒ肷��B�J�����̈ʒu�A�����̑��A�X�N���[����������B
void setCamera(DataSet* data)
{
	HRESULT hr = D3D_OK;
	// �r���[�s��쐬�̂��߁A�J�����ʒu�����߂�
	// �J�����͎����̈ʒu�A���Ă���_�i�����_�j�A�p���̂R�v�f����Ȃ�B
	///+ input
	///+ input
	D3DXVECTOR3 camUp(0, 1, 0); // �J�����͐����p�� up=(0, 1, 0)

	// �J�����ʒu��񂩂�s����쐬����.
	D3DXMATRIX camera;
	///+ input

	// �r���[�s��Ƃ��Đݒ肷��.
	///+ input

	// ���e�s���ݒ肷��B�X�N���[���ɓ��e���邽�߂ɕK�v�B����p�̓�/2�Ƃ���B
	D3DXMATRIX proj;
	///+ input

	// �쐬�����s����A���e�s��ɐݒ�
	///+ input
}

// ���C�g����ݒ肷��B�O���Ɩ���p����B
void setLight(DataSet* data)
{
	// ���C�g�̌��ʂƈʒu�����߁A0�Ԃ̃��C�g�Ƃ��Đݒ�A�L���ɂ���B
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;		// �f�B���N�V���i�����C�g

	// �L�[���C�g�͋������F��.
	light.Diffuse.r = 1.0;
	light.Diffuse.g = 1.0;
	light.Diffuse.b = 1.0;
	light.Ambient.r = 0.2f;
	light.Ambient.g = 0.2f;
	light.Ambient.b = 0.2f;
	// ������(1,-1,-1)���������C�g.
	light.Direction.x = 1;
	light.Direction.y = -1;
	light.Direction.z = -1;
	// ���C�g��ݒ肷��ɂ͂ǂ�Ȋ֐����g���΂悢��.
	///+ input
	///+ input

	// �t�B�����C�g�͎ア���F��.
	light.Diffuse.r = 0.6f;
	light.Diffuse.g = 0.6f;
	light.Diffuse.b = 0.6f;
	// ������(-1, 1,-1)���������C�g�B�O�ԃ��C�g�ƍ��E�ɋt����.
	light.Direction.x = -1;
	light.Direction.y = 1;
	light.Direction.z = -1;
	data->dev->SetLight(1, &light);				// �P�Ԃɐݒ�.
	data->dev->LightEnable(1, data->light[1]);	// �P�ԃJ�����́i�L���^�����j��ݒ肷��.

	// �o�b�N���C�g�͋��߂�.
	light.Diffuse.r = 0.8f;
	light.Diffuse.g = 0.8f;
	light.Diffuse.b = 0.8f;
	// ������(-1, 1,1)���������C�g�B���f������납��Ƃ炷.
	light.Direction.x = -1;
	light.Direction.y = -1;
	light.Direction.z = 1;
	data->dev->SetLight(2, &light);				// �Q�Ԃɐݒ�.
	data->dev->LightEnable(2, data->light[2]);	// �Q�ԃJ�����́i�L���^�����j��ݒ肷��.
}

////////////////////////// ::updateData //////////////////////////

// �f�[�^���A�b�v�f�[�g����B
// param data �f�[�^�Z�b�g
void updateData(DataSet* data)
{}

// DataSet�Ɋ�Â��āA�X�v���C�g��`��
// param data �f�[�^�Z�b�g
// return ���������G���[
HRESULT drawData(DataSet* data)
{
	// �w�i�F�����߂�BRGB=(200,200,255)�Ƃ���B
	D3DCOLOR rgb = D3DCOLOR_XRGB(200, 200, 250);
	// ��ʑS�̂������B
	data->dev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, rgb, 1.0f, 0);

	// �`����J�n�i�V�[���`��̊J�n�j
	data->dev->BeginScene();

	// �J��������ݒ肷��
	setCamera(data);
	// ���C�g����ݒ肷��
	setLight(data);

	// ���[���h���W�n�̐ݒ�
	D3DXMATRIX ww, tmp;
	///+ input
	// Y�����S�̉�]���s��ŕ\�����Aww�s��ƐώZ
	///+ input
	///+ input
	// X�����S�̉�]���s��ŕ\�����Aww�s��ƐώZ
	D3DXMatrixRotationX(&tmp, data->rotx);
	D3DXMatrixMultiply(&ww, &ww, &tmp);
	// ���[���h���W�Ƃ��Đݒ肷��B����ȍ~�`�悳���I�u�W�F�N�g�͂��̍s��̉e�����󂯂�
	///+ input

	// X�t�@�C���f�[�^�̕`��
	data->xdata->draw(data);

	data->dev->EndScene();

	// ���ۂɉ�ʂɕ\���B�o�b�N�o�b�t�@����t�����g�o�b�t�@�ւ̓]��
	// �f�o�C�X�������̃t���O�w��ɂ��A������VSYNC��҂B
	data->dev->Present(NULL, NULL, NULL, NULL);

	// Direct3D�\����AGDI�ɂ�蕶����\��
	HDC hdc = GetDC(data->hWnd);
	SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
	SetBkMode(hdc, TRANSPARENT);
	TCHAR text[256] = { 0 };
	_stprintf_s(text, _countof(text), _TEXT("LIGHT %d%d%d, rot=(%.2f, %.2f) z=%.2f"),
			  data->light[0], data->light[1], data->light[2],
			  data->rotx, data->roty, data->transz);
	TextOut(hdc, 20, 700, text, (int)_tcsclen(text));		// ������̕\��
	ReleaseDC(data->hWnd, hdc);

	return D3D_OK;
}


// �C�x���g�����R�[���o�b�N�i�E�B���h�E�v���V�[�W���j�B
// �C�x���g��������DispatchMessage�֐�����Ă΂��
// param hWnd �C�x���g�̔��������E�B���h�E
// param uMsg �C�x���g�̎�ނ�\��ID
// param wParam ��ꃁ�b�Z�[�W�p�����[�^
// param lParam ��񃁃b�Z�[�W�p�����[�^
// return DefWindowProc�̖߂�l�ɏ]��
LRESULT CALLBACK
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �}�E�X�ʒu�L�^�p�ϐ�
	static int mxx, myy;
	// �C�x���g�̎�ނɉ����āAswitch���ɂď�����؂蕪����B
	switch (uMsg)
	{
		case WM_KEYDOWN:
		// ESC�L�[���������ꂽ��I��
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		if (wParam == '1') mydata.light[0] = !mydata.light[0];
		if (wParam == '2') mydata.light[1] = !mydata.light[1];
		if (wParam == '3') mydata.light[2] = !mydata.light[2];
		break;
		case WM_SIZE:
		mydata.aspect = (float)LOWORD(lParam) / (float)HIWORD(lParam);
		break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		// �{�^���������ꂽ��ʒu���L�^���Ă����B
		mxx = MAKEPOINTS(lParam).x;
		myy = MAKEPOINTS(lParam).y;
		break;

		case WM_MOUSEMOVE:
		{
			int xx = MAKEPOINTS(lParam).x;
			int yy = MAKEPOINTS(lParam).y;
			// ���{�^�������ŁA�|���S����]��
			if (wParam & MK_LBUTTON)
			{
				mydata.roty -= (mxx - xx) * 0.01f;
				mydata.rotx -= (myy - yy) * 0.01f;
			}
			// �E�{�^�������ŁA�|���S���̑O��ړ���
			if (wParam & MK_RBUTTON)
			{
				mydata.transz += (myy - yy) * 0.1f;
			}
			mxx = xx; myy = yy;
		}
		break;

		case WM_CLOSE:		// �I���ʒm(CLOSE�{�^���������ꂽ�ꍇ�Ȃ�)���͂����ꍇ
		// �v���O�������I�������邽�߁A�C�x���g���[�v��0��ʒm����B
		// ���̌��ʁAGetMessage�̖߂�l��0�ɂȂ�B
		PostQuitMessage(0);
		break;
		default:
		break;
	}

	// �f�t�H���g�̃E�B���h�E�C�x���g����
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Window���쐬����B
// return �E�B���h�E�n���h��
HWND initWindow(DataSet* data)
{
	// �܂��E�B���h�E�N���X��o�^����B
	// ����̓E�B���h�E������̏����̎d����Windows�ɋ����邽�߂ł���B
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));	// �ϐ�wc���[���N���A����
	wc.cbSize = sizeof(WNDCLASSEX);			// ���̍\���̂̑傫����^����
	wc.lpfnWndProc = (WNDPROC)WindowProc;	// �E�B���h�E�v���V�[�W���o�^
	wc.hInstance = data->hInstance;				// �C���X�^���X�n���h����ݒ�
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);	// �}�E�X�J�[�\���̓o�^
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);	// �w�i��GRAY��
	wc.lpszClassName = wndClsName;	// �N���X���ACreateWindow�ƈ�v������
	RegisterClassEx(&wc);			// �o�^

	// �E�B���h�E���쐬����B�N���X����"directx"
	data->hWnd = CreateWindow(wndClsName, version, WS_OVERLAPPEDWINDOW,
							  0, 0, WIDTH, HEIGHT, NULL, NULL, data->hInstance, NULL);

	return data->hWnd;
}

// ���C�����[�v
// param hInstance �A�v���P�[�V������\���n���h��
void MainLoop(DataSet* data)
{
	HRESULT hr = E_FAIL;
	data->hWnd = initWindow(data);			// �E�B���h�E���쐬����

	// Direct3D������������
	hr = initDirect3D(data);
	if (FAILED(hr))
	{
		MessageBox(NULL, d3dErrStr(hr), _TEXT("Direct3D���������s"), MB_OK);
		return;
	}
	// �e��3D�`��ݒ���s��
	data->dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	// �J�����O����
	data->dev->SetRenderState(D3DRS_LIGHTING, TRUE);	// ���C�g�L��
	data->dev->SetRenderState(D3DRS_ZENABLE, TRUE);		// Z�o�b�t�@�����O�L��
	// ���C�gON
	data->light[0] = data->light[1] = data->light[2] = TRUE;

	// torus.x��ǂݍ��ށB
	data->xdata = MyXData::load(&mydata, _TEXT("data/torus.x"));
	data->transz = 50;

	// �A�X�y�N�g���������.
	data->aspect = (float)WIDTH / (float)HEIGHT;

	ShowWindow(data->hWnd, SW_SHOWNORMAL);	// �쐬�����E�B���h�E��\������B

	// �C�x���g���[�v�B
	// �u���b�N�^�֐�GetMessage�ł͂Ȃ��m���u���b�N�^�֐���PeekMessage���g���B
	MSG msg;
	bool flag = 1;
	while (flag)
	{
		// ���b�Z�[�W�����邩�ǂ����m�F����
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			// ���b�Z�[�W������̂ŏ�������
			if (GetMessage(&msg, NULL, 0, 0) == 1)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				flag = 0;
			}
		}
		else
		{
			updateData(data);	// �ʒu�̍Čv�Z
			drawData(data);		// �`��
		}
		// Sleep�ȂǍs���Ă��Ȃ����AdrawData�֐�����data->dev->Present��
		// ��ʍX�V�҂����s���̂ŁA���ʓI��1/60���ƂɃC�x���g���[�v�����B
	}
}

#define RELEASE(__xx__) if (__xx__) { __xx__->Release(); __xx__ = 0; }

// DataSet���������B
void ReleaseDataSet(DataSet* data)
{
	if (data->xdata) delete data->xdata;
	RELEASE(data->dev);
	RELEASE(data->pD3D);
}


// param argc �R�}���h���C������n���ꂽ�����̐�
// param argv �����̎��̂ւ̃|�C���^�z��
// return 0 ����I��
int main(int argc, char* argv[])
{
	// ���̃v���O���������s�����Ƃ��̃C���X�^���X�n���h�����擾
	mydata.hInstance = GetModuleHandle(NULL);
	MainLoop(&mydata);

	// �m�ۂ������\�[�X�����
	ReleaseDataSet(&mydata);
	return 0;
}


// Direct3D������������
// param data �f�[�^�Z�b�g
// return ���������G���[�܂���D3D_OK
HRESULT initDirect3D(DataSet* data)
{
	HRESULT hr;

	// Direct3D�C���X�^���X�I�u�W�F�N�g�𐶐�����B
	// D3D_SDK_VERSION�ƁA�����^�C���o�[�W�����ԍ����K�؂łȂ��ƁANULL���Ԃ�B
	data->pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	// NULL�Ȃ烉���^�C�����s�K��
	if (data->pD3D == NULL) return E_FAIL;

	// PRESENT�p�����[�^���[���N���A���A�K�؂ɏ�����
	ZeroMemory(&(data->d3dpp), sizeof(data->d3dpp));
	// �E�B���h�E���[�h��
	data->d3dpp.Windowed = TRUE;
	// Direct3D�ɃX���b�v�G�t�F�N�g��C����
	data->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// �o�b�N�o�b�t�@��RGB���ꂼ��W�r�b�g�ŁB
	data->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Present���ɐ��������ɍ��킹��
	data->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// 3D�p��Z�o�b�t�@(depth buffer)���쐬
	data->d3dpp.EnableAutoDepthStencil = TRUE;		// Direct3D�ɐ[�x�o�b�t�@�̊Ǘ���C����
	data->d3dpp.AutoDepthStencilFormat = D3DFMT_D16;// �[�x�o�b�t�@�̃t�H�[�}�b�g�i�ʏ�͂��̒l�Ŗ��Ȃ��j

	// D3D�f�o�C�X�I�u�W�F�N�g�̍쐬�B
	hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
								  D3DCREATE_HARDWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	if (FAILED(hr))
	{
		// D3D�f�o�C�X�I�u�W�F�N�g�̍쐬�BHAL&SOFT
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
									  D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
		if (FAILED(hr))
		{
			// D3D�f�o�C�X�I�u�W�F�N�g�̍쐬�BREF&SOFT
			hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, data->hWnd,
										  D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
		}
	}
	if (FAILED(hr)) return hr;

	return D3D_OK;
}

// �G���[��������HRESULT�𕶎���ɕϊ����邽�߂̕⏕�֐�
const TCHAR* d3dErrStr(HRESULT res)
{
	switch (res)
	{
		case D3D_OK: return _TEXT("D3D_OK");
		case D3DERR_DEVICELOST: return _TEXT("D3DERR_DEVICELOST");
		case D3DERR_DRIVERINTERNALERROR: return _TEXT("D3DERR_DRIVERINTERNALERROR");
		case D3DERR_INVALIDCALL: return _TEXT("D3DERR_INVALIDCALL");
		case D3DERR_OUTOFVIDEOMEMORY: return _TEXT("D3DERR_OUTOFVIDEOMEMORY");
		case D3DERR_DEVICENOTRESET: return _TEXT("D3DERR_DEVICENOTRESET");
		case D3DERR_NOTAVAILABLE: return _TEXT("D3DERR_NOTAVAILABLE");
		case D3DXERR_INVALIDDATA: return _TEXT("D3DXERR_INVALIDDATA");
		case E_OUTOFMEMORY: return _TEXT("E_OUTOFMEMORY");
	}
	return _TEXT("unknown error");
}
