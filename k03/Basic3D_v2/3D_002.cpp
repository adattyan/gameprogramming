/*
	3D_002.cpp

	�v���~�e�B�u

	���ށuGP3_3D��bv2.pptx�v���Q�l�ɂ��� input �����ɕK�v�ȃR�[�h��ǉ�����
	�v���O���������������Ă��������B
*/
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <d3d9.h>			// Direct3D9 (DirectX9 Graphics)�ɕK�v
#include <d3dx9core.h>		// DirectX �X�v���C�g�ɕK�v

// �E�B���h�E�̕��ƍ�����萔�Őݒ肷��
#define WIDTH 1024			//< ��
#define HEIGHT 768			//< ����

// �E�B���h�E�^�C�g���o�[�ɕ\�������o�[�W������
static const TCHAR version[] = _TEXT(__DATE__ " " __TIME__);

// �E�B���h�E�N���X��.
static const TCHAR wndClsName[] = _TEXT("directx");

// �N���X�錾�B���̂̒�`����ɃN���X��������錾���Ă����B
struct DataSet;

// �֐��錾
HRESULT initDirect3D(DataSet* data);
const TCHAR* d3dErrStr(HRESULT res);
void ReleaseDataSet(DataSet* data);

// ���_�t�H�[�}�b�g�\���̂��`����
// ���_�́A���W�Ɩ@���x�N�g���A�|�C���g�T�C�Y�A�F�����܂ށB
// �����̃����o�v�f�̕��я��ɂ͌��܂肪����A���Ԃ��ԈႦ��Ɛ������\�����ł��Ȃ��B
struct MyVertex
{
	///+ input
	FLOAT nx, ny, nz;	// �@���x�N�g��
	FLOAT pt;			// �|�C���g�T�C�Y
	///+ input
	FLOAT tu, tv;		// �e�N�X�`�����W
};

// ���_�\���̂ƈ�v����悤�AD3DFVF���`����B
// D3DFVF_XYZ �E�E�E���_���W�ʒu
// D3DFVF_NORMAL �E�E�E�@���x�N�g��
// D3DFVF_PSIZE �E�E�E���_�̃|�C���g�T�C�Y�i�_�̌������̑傫���j
// D3DFVF_DIFFUSE �E�E�E���_�̐F
// D3DFVF_TEX1 �E�E�E�e�N�X�`��UV���W
#define kMYFVF (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_PSIZE|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// ���_�̈ʒu�͂R�c��Ԃ̃��[���h���W�n�i�X�N���[�����W�n�ł͂���܂���j
// �\����MyVertex�ɏ]���āA���_���`����B
// ��ŃC���f�b�N�X�w�肷��̂ŁA���_�z��ł̊e���_�̕��я��͓K���ō\��Ȃ��B
static MyVertex vertices[] = {
	{ -10.0f, -10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff), 0.0f, 1.0f },	// ���_0
	{ -10.0f,  10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0xff, 0x00, 0x00, 0xff), 0.0f, 0.0f },	// ���_1
	{  10.0f, -10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0x00, 0xff, 0x00, 0xff), 1.0f, 1.0f },	// ���_2
	{  10.0f,  10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0x00, 0x00, 0xff, 0xff), 1.0f, 0.0f },	// ���_3
};

// �C���f�b�N�X���X�g���쐬����B���̔z��͎O�p�`�̒��_�̑g�ݍ��킹��
// �������̂ł���B�����͒��_�z��ł̃C���f�b�N�X�ʒu�������A�R����
// �P�̎O�p�`�ɂȂ�B
// �Q�̎O�p�`�̒��_�͈ȉ��̂悤�ɂȂ��Ă���B
//  1      1 -- 3
//  | �_     �_ |
//  0 -- 2      2
// ���̂��߁A�C���f�b�N�X���X�g�́A(0, 1, 2), (1, 3, 2)�ƂȂ�B
// ���̐��l���т́A���_�̏��Ԃ���������(���v���)�ɂȂ��Ă��邱�Ƃɒ��ӁB
static short indices[] = {
	///+ input
};

// �������������߂̒��_�\���́BMyVertex�ƈقȂ�A���W�_�ƃJ���[�݂̂�錾�B
struct MyLineVertex
{
	FLOAT x, y, z;
	DWORD color;
};

// MyLineVertex�p��FVF
#define kXYZCOL (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// ���W���\���̂��߂̃f�[�^
static MyLineVertex axis[] = {
	{  30.0f, 0.0f, 0.0f,  D3DCOLOR_RGBA(0xff, 0x00, 0x00, 0xff), },
	{ -30.0f, 0.0f, 0.0f,  D3DCOLOR_RGBA(0xff, 0x00, 0x00, 0xff), },
	{ 0.0f,  30.0f, 0.0f,  D3DCOLOR_RGBA(0x00, 0xff, 0x00, 0xff), },
	{ 0.0f, -30.0f, 0.0f,  D3DCOLOR_RGBA(0x00, 0xff, 0x00, 0xff), },
	{ 0.0f, 0.0f,  30.0f,  D3DCOLOR_RGBA(0x00, 0x00, 0xff, 0xff), },
	{ 0.0f, 0.0f, -30.0f,  D3DCOLOR_RGBA(0x00, 0x00, 0xff, 0xff), },
};

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
	bool light[4];				//< ���C�g��ON/OFF�p�t���O
	// �e��t���O
	bool flag[10];				//< 0:SOLID/WIRE

	IDirect3DVertexBuffer9* vb;	//< ���_�o�b�t�@
	IDirect3DIndexBuffer9* ib;	//< �C���f�b�N�X�o�b�t�@
	IDirect3DTexture9* tex;		//< �e�N�X�`��

	float cam_rotx;				//< �J������]��
	float cam_roty;
} mydata;

void resetData(DataSet* data)
{
	data->cam_rotx = data->cam_roty = 0;
	data->rotx = data->roty = 0;
}

////////////////////////// ::setCamera //////////////////////////
// �J������ݒ肷��B�J�����̈ʒu�A�����̑��A�X�N���[����������B
void setCamera(DataSet* data)
{
	HRESULT hr = D3D_OK;
	// �r���[�s��쐬�̂��߁A�J�����ʒu�����߂�
	// �J�����͎����̈ʒu�A���Ă���_�i�����_�j�A�p���̂R�v�f����Ȃ�B
	D3DXVECTOR3 cam(0, 0, data->transz);	// �J�����ʒu�� (0, 0, transz)
	D3DXVECTOR3 camAt(0, 0, 0); // �J�����̒����_�͌��_
	D3DXVECTOR3 camUp(0, 1, 0); // �J�����͐����p�� up=(0, 1, 0)

	// �J�����ʒu��񂩂�s����쐬�BRH�͉E����W�n�̈Ӗ�
	D3DXMATRIX camera;
	D3DXMatrixLookAtRH(&camera, &cam, &camAt, &camUp);

	// �J��������]������BSHIFT�L�[�{�}�E�X���{�^��
	D3DXMATRIX mx, my;
	// X����Y�����S�̉�]���s��ŕ\��
	D3DXMatrixRotationY(&my, data->cam_roty);
	D3DXMatrixRotationX(&mx, data->cam_rotx);
	// camera�s��Ƃ����Z�B
	///+ input

	// �r���[�s��Ƃ��Đݒ�
	data->dev->SetTransform(D3DTS_VIEW, &camera);

	// ���e�s���ݒ肷��B�X�N���[���ɓ��e���邽�߂ɕK�v�B����p�̓�/2
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovRH(&proj, D3DX_PI / 2, data->aspect, 0.1f, 200.0f);
	// �쐬�����s����A���e�s��ɐݒ�
	data->dev->SetTransform(D3DTS_PROJECTION, &proj);
}

// ���C�g����ݒ肷��B�O���Ɩ���p����B
void setLight(DataSet* data)
{
	// ���C�g�̌��ʂƈʒu�����߁A0�Ԃ̃��C�g�Ƃ��Đݒ�A�L���ɂ���B
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;		// �f�B���N�V���i�����C�g

	// �L�[���C�g�͋������F�ɁB
	light.Diffuse.r = 1.0;
	light.Diffuse.g = 1.0;
	light.Diffuse.b = 1.0;
	light.Ambient.r = 0.2f;
	light.Ambient.g = 0.2f;
	light.Ambient.b = 0.2f;
	// ������(1,-1,-1)���������C�g�B
	light.Direction.x = 1;
	light.Direction.y = -1;
	light.Direction.z = -1;
	data->dev->SetLight(0, &light);		// 0�Ԃɐݒ�
	data->dev->LightEnable(0, data->light[0]);	// �L���ɂ���

	// �t�B�����C�g�͎ア���F�ɁB
	light.Diffuse.r = 0.6f;
	light.Diffuse.g = 0.6f;
	light.Diffuse.b = 0.6f;
	// ������(-1, 1,-1)���������C�g�B�O�ԃ��C�g�ƍ��E�ɋt����
	light.Direction.x = -1;
	light.Direction.y = 1;
	light.Direction.z = -1;
	data->dev->SetLight(1, &light);		// 1�Ԃɐݒ�
	data->dev->LightEnable(1, data->light[1]);	// �L���ɂ���

	// �o�b�N���C�g�͋��߂ɁB
	light.Diffuse.r = 0.8f;
	light.Diffuse.g = 0.8f;
	light.Diffuse.b = 0.8f;
	// ������(-1, 1,1)���������C�g�B���f������납��Ƃ炷�B
	light.Direction.x = -1;
	light.Direction.y = -1;
	light.Direction.z = 1;
	data->dev->SetLight(2, &light);		// 2�Ԃɐݒ�
	data->dev->LightEnable(2, data->light[2]);	// �L���ɂ���
}

////////////////////////// ::updateData //////////////////////////

// �f�[�^���A�b�v�f�[�g����B
// param data �f�[�^�Z�b�g
void updateData(DataSet* data)
{}


////////////////////////// ::drawData //////////////////////////
// DataSet�Ɋ�Â��āA�X�v���C�g��`��
// param data �f�[�^�Z�b�g
// return ���������G���[
HRESULT drawData(DataSet* data)
{
	HRESULT hr = D3D_OK;

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

	// �f�t�H���g�̓��C�e�B���OOFF
	data->dev->SetRenderState(D3DRS_LIGHTING, data->light[3]);
	if (data->flag[0] == false)
	{
		data->dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	else
	{
		data->dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}

	// ���[���h���W�n�̐ݒ�
	D3DXMATRIX ww;
	D3DXMatrixIdentity(&ww);		// �܂��P�ʍs��ɂ���
	D3DXMATRIX tmp;
	// Y�����S�̉�]���s��ŕ\�����Aww�s��ƐώZ
	D3DXMatrixRotationY(&tmp, data->roty);
	D3DXMatrixMultiply(&ww, &ww, &tmp);
	// X�����S�̉�]���s��ŕ\�����Aww�s��ƐώZ
	D3DXMatrixRotationX(&tmp, data->rotx);
	D3DXMatrixMultiply(&ww, &ww, &tmp);
	// ���[���h���W�Ƃ��Đݒ肷��B����ȍ~�`�悳���I�u�W�F�N�g�͂��̍s��̉e�����󂯂�
	data->dev->SetTransform(D3DTS_WORLD, &ww);

	// FVF���w��
	data->dev->SetFVF(kMYFVF);
	// ���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@���X�g���[���ɃZ�b�g
	///+ input
	///+ input
	// �e�N�X�`���̎w���OFF��
	if (data->flag[1])
	{
		data->dev->SetTexture(0, data->tex);
	}
	else
	{
		data->dev->SetTexture(0, NULL);
	}
	// �g���C�A���O���E���X�g�ł�PSIZE�͖��������
	///+ input

	// ����ȍ~�̓��C�e�B���OOFF�A�e�N�X�`��OFF��
	data->dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	data->dev->SetTexture(0, NULL);

	// ���_��`��(D3DPT_POINTLIST).
	// DIFFUSE���ז��Ȃ̂ŁAFVF��ύX�B�\���̂̕��тɖ������Ȃ����FVF��
	// �ύX�ł���B�܂�A�\���̂̓r���̃����o���O�����Ƃ͂ł��Ȃ����A
	// �Ō�̃����o�ł���Ζ������邱�Ƃ��\�B
	// DIFFUSE���L���̂܂܂��ƁA���_�J���[�������Ă��܂��B
	data->dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_PSIZE);
	data->dev->DrawPrimitive(D3DPT_POINTLIST, 0, 4);

	// ���W����`��(D3DPT_LINELIST)
	D3DXMatrixIdentity(&ww);
	data->dev->SetTransform(D3DTS_WORLD, &ww);
	// FVF��ύX�B���_�o�b�t�@�̍\���ɍ��킹��B
	data->dev->SetFVF(kXYZCOL);
	data->dev->DrawPrimitiveUP(D3DPT_LINELIST, 3, (VOID*)axis, sizeof(MyLineVertex));

	data->dev->EndScene();

	// ���ۂɉ�ʂɕ\���B�o�b�N�o�b�t�@����t�����g�o�b�t�@�ւ̓]��
	// �f�o�C�X�������̃t���O�w��ɂ��A������VSYNC��҂B
	data->dev->Present(NULL, NULL, NULL, NULL);

	// Direct3D�\����AGDI�ɂ�蕶����\��
	HDC hdc = GetDC(data->hWnd);
	SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
	SetBkMode(hdc, TRANSPARENT);
	TCHAR text[256] = { 0 };
	_stprintf_s(text, _countof(text), _TEXT("LIGHT %s %d%d%d, cam rot=(%.2f, %.2f) z=%.2f rot=(%.2f, %.2f) flag %d"),
			  data->light[3] ? _TEXT("ON") : _TEXT("OFF"), data->light[0], data->light[1], data->light[2],
			  data->cam_rotx, data->cam_roty, data->transz, data->rotx, data->roty,
			  data->flag[0]);

	static int text_posx = 20;
	static int text_posy = 0;
	if (text_posy == 0)
	{
		SIZE sz;
		GetTextExtentPoint32(hdc, text, (int)_tcsclen(text), &sz);

		text_posy = HEIGHT - sz.cy - 20;
	}
	TextOut(hdc, text_posx, text_posy, text, (int)_tcsclen(text));// ������̕\��
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
		if (wParam == '4') mydata.light[3] = !mydata.light[3];
		if (wParam == 'W') mydata.flag[0] = !mydata.flag[0];
		if (wParam == 'T') mydata.flag[1] = !mydata.flag[1];
		if (wParam == 'R') resetData(&mydata);
		break;
		case WM_SIZE:
		mydata.aspect = LOWORD(lParam) / (float)HIWORD(lParam);
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
				if (wParam & MK_SHIFT)
				{
					mydata.cam_roty -= (mxx - xx) * 0.01f;
					mydata.cam_rotx -= (myy - yy) * 0.01f;
				}
				else
				{
					mydata.roty -= (mxx - xx) * 0.01f;
					mydata.rotx -= (myy - yy) * 0.01f;
				}
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

	RECT rc = { 0, 0, WIDTH, HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// �E�B���h�E���쐬����B�N���X����"directx"
	data->hWnd = CreateWindow(wndClsName, version, WS_OVERLAPPEDWINDOW,
							  0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, data->hInstance, NULL);

	return data->hWnd;
}

// �e�N�X�`��fname��ǂݍ��ށB�G���[�������������O�𔭐�����B
// param fname �t�@�C�����Bconst�C���q�����֐����Œl��ύX���Ȃ����Ƃ�錾����B
// param tex �쐬����e�N�X�`���ւ̃|�C���^�����邽�߂̃|�C���^�B
HRESULT loadTexture(IDirect3DDevice9* dev, const TCHAR fname[], IDirect3DTexture9** tex)
{
	HRESULT hr = D3DXCreateTextureFromFile(dev, fname, tex);
	if (FAILED(hr))
	{
		MessageBox(NULL, _TEXT("�e�N�X�`���ǂݍ��ݎ��s"), fname, MB_OK);
		throw hr;		// �G���[�����������̂ŗ�O�𓊂���.
	}
	return hr;
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
	data->dev->SetRenderState(D3DRS_ZENABLE, TRUE);		// Z�o�b�t�@�����O�L��
	// ���C�gON
	data->light[0] = data->light[1] = data->light[2] = TRUE;
	data->aspect = (float)WIDTH / (float)HEIGHT;
	data->rotx = data->roty = 0.0f;
	data->transz = 50.0f;

	// ���_�o�b�t�@���쐬�Bsizeof(vertices)�o�C�g���̃��������m�ۂ���B
	// �o�b�t�@�̓r�f�I�������ɍ���邽�߁A�����ȕ`�悪���҂ł���B
	hr = data->dev->CreateVertexBuffer(sizeof(vertices), 0, kMYFVF,
									   D3DPOOL_DEFAULT, &(data->vb), NULL);

	// ���_�o�b�t�@�����b�N���āA�|�C���^�A�h���X���擾����B
	// ���b�N����ƃ��C���������Ƀf�[�^���z�u�����B
	// ���̌㒸�_�z��(vertices)���R�s�[���A�����Ƀ��b�N��������B
	// ���b�N��������ɂ��AVRAM�Ƀf�[�^���]�������B
	VOID* pV;
	///+ input
	///+ input
	///+ input

	// �C���f�b�N�X�o�b�t�@�̍쐬�ƃf�[�^�R�s�[
	hr = data->dev->CreateIndexBuffer(sizeof(indices), 0, D3DFMT_INDEX16,
									  D3DPOOL_DEFAULT, &(data->ib), NULL);
	VOID* pI;
	hr = data->ib->Lock(0, sizeof(indices), &pI, 0);
	CopyMemory(pI, indices, sizeof(indices));
	data->ib->Unlock();

	// �e�N�X�`���摜��ǂݍ���
	loadTexture(data->dev, _TEXT("data/koara.jpg"), &(data->tex));

	ShowWindow(data->hWnd, SW_SHOWNORMAL);	// �쐬�����E�B���h�E��\������B

	// �����v���̒��_�����w�ʂ��J�����O
	//data->dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	// �J�����O�𖳌���
	data->dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

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
	RELEASE(data->ib);
	RELEASE(data->vb);
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
	data->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// �o�b�N�o�b�t�@��RGB���ꂼ��W�r�b�g�ŁB
	data->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Present���ɐ��������ɍ��킹��
	data->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// 3D�p��Z�o�b�t�@(depth buffer)���쐬
	data->d3dpp.EnableAutoDepthStencil = TRUE;
	data->d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

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
