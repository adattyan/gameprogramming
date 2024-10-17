/*
	3D_001_expe_1.cpp

	はじめての3D（Xファイル読み込み）

	実験①	sample.xを読み込み表示してください。
*/
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <d3d9.h>			// Direct3D9 (DirectX9 Graphics)に必要
#include <d3dx9core.h>		// DirectX スプライトに必要
#include <tchar.h>
#include <assert.h>

// ウィンドウの幅と高さを定数で設定する
#define WIDTH 1024			//< 幅
#define HEIGHT 768			//< 高さ

// ウィンドウタイトルバーに表示されるバージョン名
static const TCHAR version[] = _TEXT(__DATE__ " " __TIME__);

// ウィンドウクラス名.
static const TCHAR wndClsName[] = _TEXT("directx");

// クラス宣言。実体の定義より先にクラス名だけを宣言しておく。
struct DataSet;
class MyXData;

// 関数宣言
HRESULT initDirect3D(DataSet* data);
const TCHAR* d3dErrStr(HRESULT res);
void ReleaseDataSet(DataSet* data);

// プログラムに必要な変数を構造体として定義
struct DataSet
{
	HINSTANCE hInstance;		//< インスタンスハンドル
	HWND hWnd;					//< 表示ウィンドウ
	IDirect3D9* pD3D;			//< Direct3Dインスタンスオブジェクト
	D3DPRESENT_PARAMETERS d3dpp;//< デバイス作成時のパラメータ
	IDirect3DDevice9* dev;		//< Direct3Dデバイスオブジェクト

	// 3D表示用のデータ
	float aspect;				//< 画面アスペクト比（縦横比）
	float rotx, roty;			//< オブジェクトを回転させる量
	float transz;				//< カメラのZ位置

	// ライトのON/OFF
	bool light[3];				//< ライトのON/OFF用フラグ

	MyXData* xdata;				//< Xファイルのデータ
} mydata;

////////////////////////// class MyXData //////////////////////////
// Xデータを管理するクラス。
// メンバ関数にloadがあるが、この関数はstatic関数。つまり、クラス関数。
// インスタンスを生成するための関数となっている。
class MyXData
{
private:
	// Xファイル読み込みのために必要な変数
	ID3DXMesh* pMesh_;				//< 形状データ
	DWORD numMaterials_;			//< マテリアルデータ数
	IDirect3DTexture9** pTextures_;	//< テクスチャ情報
	D3DMATERIAL9* pMaterials_;		//< マテリアルデータ

public:
	MyXData() :pMesh_(0), numMaterials_(0), pTextures_(0), pMaterials_(0) {}
	~MyXData();
	void draw(DataSet* data);		//< 描画

	// staticメンバ関数は、クラス関数となる。
	static MyXData* load(DataSet* data, const TCHAR* fname);
};

// デストラクタでは全データを解放する
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

// Xファイルを読み込む。自分自身を生成するため、staticメンバ関数となっている。
// arg data データセット
// arg fname ファイル名
MyXData* MyXData::load(DataSet* data, const TCHAR* fname)
{
	MyXData* xxx = new MyXData();		// インスタンスを生成

	// 指定するXファイルを読み込んで、システムメモリに展開する。
	// ポリゴンメッシュ情報の他に、マテリアル情報も読み込む。
	HRESULT hr;
	ID3DXBuffer* pMB;		// マテリアル情報のためのデータバッファ

	// D3DXLoadMeshFromXによりXファイルからポリゴンメッシュを読み込む。
	// データ配置場所としてSYSTEMMEM(システムメモリ)を選択する。
	// これによりフルスクリーン切り替え時にもデータは破壊されない。
	hr = D3DXLoadMeshFromX(fname, D3DXMESH_SYSTEMMEM, data->dev, NULL,
						   &pMB, NULL, &xxx->numMaterials_, &xxx->pMesh_);
	if (hr != D3D_OK)
	{
		MessageBox(NULL, _TEXT("Xファイルの読み込みに失敗"), fname, MB_OK);
		return 0;
	}

	// マテリアル情報とテクスチャ情報を取得し、メインメモリに保存しておく。
	// マテリアル数＝テクスチャ数という仮定ができるので、numMaterials_の
	// 数だけ配列を作成する。
	xxx->pMaterials_ = new D3DMATERIAL9[xxx->numMaterials_];
	xxx->pTextures_ = new IDirect3DTexture9 * [xxx->numMaterials_];

	// マテリアル情報にアクセスするため、ポインタを得る。
	// ポインタは汎用型なので、マテリアル型に変換しなければならない。
	D3DXMATERIAL* mat = (D3DXMATERIAL*)pMB->GetBufferPointer();

	// マテリアルは複数個ありうるので、その全てについて、情報を取得
	for (unsigned int ii = 0; ii < xxx->numMaterials_; ii++)
	{
		// MatD3Dはマテリアル（色）情報が格納されている構造体。
		xxx->pMaterials_[ii] = mat[ii].MatD3D;
		if (mat[ii].pTextureFilename) {
			// テクスチャデータを読み込む。
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
			// 読み込み失敗した場合はテクスチャ無し
			if (hr != D3D_OK) {
				xxx->pTextures_[ii] = NULL;
			}
		}
		else {
			xxx->pTextures_[ii] = NULL;
		}
	}
	// マテリアル情報を解放
	pMB->Release();

	return xxx;
}

// Xデータを描画する
void MyXData::draw(DataSet* data)
{
	for (unsigned int ii = 0; ii < this->numMaterials_; ii++)
	{
		// テクスチャがあれば、設定する
		data->dev->SetTexture(0, this->pTextures_[ii]);
		// マテリアル情報を設定し、ポリゴンに色をつける
		data->dev->SetMaterial(&(this->pMaterials_[ii]));

		// ポリゴンメッシュを描画
		this->pMesh_->DrawSubset(ii);
	}
}

////////////////////////// ::setCamera //////////////////////////
// カメラを設定する。カメラの位置、向きの他、スクリーン情報もある。
void setCamera(DataSet* data)
{
	HRESULT hr = D3D_OK;
	// ビュー行列作成のため、カメラ位置を決める
	// カメラは自分の位置、見ている点（注視点）、姿勢の３要素からなる。
	D3DXVECTOR3 eye(0, 0, data->transz);	// カメラ位置は (0, 0, transz)
	D3DXVECTOR3 look(0, 0, 0);				// カメラの注視点は原点
	D3DXVECTOR3 camUp(0, 1, 0);				// カメラは垂直姿勢 up=(0, 1, 0)

	// カメラ位置情報から行列を作成する.
	D3DXMATRIX camera;
	D3DXMatrixLookAtRH(&camera, &eye, &look, &camUp);// RHは右手座標系の意味.

	// ビュー行列として設定する.
	data->dev->SetTransform(D3DTS_VIEW, &camera);

	// 投影行列を設定する。スクリーンに投影するために必要。視野角はπ/2とする。
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovRH(&proj,
							   D3DX_PI / 2,	// 視野角
							   data->aspect,// アスペクト比
							   0.1f,		// ニアクリップ距離
							   100.0f);	// ファークリップ距離

	// 作成した行列を、投影行列に設定
	data->dev->SetTransform(D3DTS_PROJECTION, &proj);
}

// ライト情報を設定する。三灯照明を用いる。
void setLight(DataSet* data)
{
	// ライトの光量と位置を決め、0番のライトとして設定、有効にする。
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;		// ディレクショナルライト

	// キーライトは強い白色に.
	light.Diffuse.r = 1.0;
	light.Diffuse.g = 1.0;
	light.Diffuse.b = 1.0;
	light.Ambient.r = 0.2f;
	light.Ambient.g = 0.2f;
	light.Ambient.b = 0.2f;
	// 方向は(1,-1,-1)を向くライト.
	light.Direction.x = 1;
	light.Direction.y = -1;
	light.Direction.z = -1;
	data->dev->SetLight(0, &light);				// ０番に設定.
	data->dev->LightEnable(0, data->light[0]);	// ０番カメラの（有効／無効）を設定する.

	// フィルライトは弱い白色に.
	light.Diffuse.r = 0.6f;
	light.Diffuse.g = 0.6f;
	light.Diffuse.b = 0.6f;
	// 方向は(-1, 1,-1)を向くライト。０番ライトと左右に逆方向.
	light.Direction.x = -1;
	light.Direction.y = 1;
	light.Direction.z = -1;
	data->dev->SetLight(1, &light);				// １番に設定.
	data->dev->LightEnable(1, data->light[1]);	// １番カメラの（有効／無効）を設定する.

	// バックライトは強めに.
	light.Diffuse.r = 0.8f;
	light.Diffuse.g = 0.8f;
	light.Diffuse.b = 0.8f;
	// 方向は(-1, 1,1)を向くライト。モデルを後ろから照らす.
	light.Direction.x = -1;
	light.Direction.y = -1;
	light.Direction.z = 1;
	data->dev->SetLight(2, &light);				// ２番に設定.
	data->dev->LightEnable(2, data->light[2]);	// ２番カメラの（有効／無効）を設定する.
}

////////////////////////// ::updateData //////////////////////////

// データをアップデートする。
// param data データセット
void updateData(DataSet* data)
{}

// DataSetに基づいて、スプライトを描画
// param data データセット
// return 発生したエラー
HRESULT drawData(DataSet* data)
{
	// 背景色を決める。RGB=(200,200,255)とする。
	D3DCOLOR rgb = D3DCOLOR_XRGB(200, 200, 250);
	// 画面全体を消去。
	data->dev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, rgb, 1.0f, 0);

	// 描画を開始（シーン描画の開始）
	data->dev->BeginScene();

	// カメラ情報を設定する
	setCamera(data);
	// ライト情報を設定する
	setLight(data);

	// ワールド座標系の設定
#if 0
	D3DXMATRIX ww, tmp;
	D3DXMatrixIdentity(&ww);		// まず単位行列にする
	// Y軸中心の回転を行列で表現し、ww行列と積算
	D3DXMatrixRotationY(&tmp, data->roty);
	D3DXMatrixMultiply(&ww, &ww, &tmp);
	// X軸中心の回転を行列で表現し、ww行列と積算
	D3DXMatrixRotationX(&tmp, data->rotx);
	D3DXMatrixMultiply(&ww, &ww, &tmp);
#else
	// 一気に各軸の回転を設定するやり方
	D3DXMATRIX ww;
	D3DXMatrixIdentity(&ww);		// まず単位行列にする
	D3DXMatrixRotationYawPitchRoll(&ww, data->roty, data->rotx, 0.0f);
#endif

	// ワールド座標として設定する。これ以降描画されるオブジェクトはこの行列の影響を受ける
	data->dev->SetTransform(D3DTS_WORLD, &ww);

	// Xファイルデータの描画
	data->xdata->draw(data);

	data->dev->EndScene();

	// 実際に画面に表示。バックバッファからフロントバッファへの転送
	// デバイス生成時のフラグ指定により、ここでVSYNCを待つ。
	data->dev->Present(NULL, NULL, NULL, NULL);

	// Direct3D表示後、GDIにより文字列表示
	HDC hdc = GetDC(data->hWnd);
	SetTextColor(hdc, RGB(0xFF, 0x00, 0x00));
	SetBkMode(hdc, TRANSPARENT);
	TCHAR text[256] = { 0 };
	_stprintf_s(text, _countof(text), _TEXT("LIGHT %d%d%d, rot=(%.2f, %.2f) z=%.2f"),
			  data->light[0], data->light[1], data->light[2],
			  data->rotx, data->roty, data->transz);
	TextOut(hdc, 20, 700, text, (int)_tcsclen(text));		// 文字列の表示
	ReleaseDC(data->hWnd, hdc);

	return D3D_OK;
}


// イベント処理コールバック（ウィンドウプロシージャ）。
// イベント発生時にDispatchMessage関数から呼ばれる
// param hWnd イベントの発生したウィンドウ
// param uMsg イベントの種類を表すID
// param wParam 第一メッセージパラメータ
// param lParam 第二メッセージパラメータ
// return DefWindowProcの戻り値に従う
LRESULT CALLBACK
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// マウス位置記録用変数
	static int mxx, myy;
	// イベントの種類に応じて、switch文にて処理を切り分ける。
	switch (uMsg)
	{
		case WM_KEYDOWN:
		// ESCキーが押下されたら終了
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
		// ボタンを押されたら位置を記録しておく。
		mxx = MAKEPOINTS(lParam).x;
		myy = MAKEPOINTS(lParam).y;
		break;

		case WM_MOUSEMOVE:
		{
			int xx = MAKEPOINTS(lParam).x;
			int yy = MAKEPOINTS(lParam).y;
			// 左ボタン押下で、ポリゴン回転に
			if (wParam & MK_LBUTTON)
			{
				mydata.roty -= (mxx - xx) * 0.01f;
				mydata.rotx -= (myy - yy) * 0.01f;
			}
			// 右ボタン押下で、ポリゴンの前後移動に
			if (wParam & MK_RBUTTON)
			{
				mydata.transz += (myy - yy) * 0.1f;
			}
			mxx = xx; myy = yy;
		}
		break;

		case WM_CLOSE:		// 終了通知(CLOSEボタンが押された場合など)が届いた場合
		// プログラムを終了させるため、イベントループに0を通知する。
		// この結果、GetMessageの戻り値は0になる。
		PostQuitMessage(0);
		break;
		default:
		break;
	}

	// デフォルトのウィンドウイベント処理
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Windowを作成する。
// return ウィンドウハンドル
HWND initWindow(DataSet* data)
{
	// まずウィンドウクラスを登録する。
	// これはウィンドウ生成後の処理の仕方をWindowsに教えるためである。
	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));	// 変数wcをゼロクリアする
	wc.cbSize = sizeof(WNDCLASSEX);			// この構造体の大きさを与える
	wc.lpfnWndProc = (WNDPROC)WindowProc;	// ウィンドウプロシージャ登録
	wc.hInstance = data->hInstance;				// インスタンスハンドルを設定
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);	// マウスカーソルの登録
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);	// 背景をGRAYに
	wc.lpszClassName = wndClsName;	// クラス名、CreateWindowと一致させる
	RegisterClassEx(&wc);			// 登録

	// ウィンドウを作成する。クラス名は"directx"
	data->hWnd = CreateWindow(wndClsName, version, WS_OVERLAPPEDWINDOW,
							  0, 0, WIDTH, HEIGHT, NULL, NULL, data->hInstance, NULL);

	return data->hWnd;
}

// メインループ
// param hInstance アプリケーションを表すハンドル
void MainLoop(DataSet* data)
{
	HRESULT hr = E_FAIL;
	data->hWnd = initWindow(data);			// ウィンドウを作成する

	// Direct3Dを初期化する
	hr = initDirect3D(data);
	if (FAILED(hr))
	{
		MessageBox(NULL, d3dErrStr(hr), _TEXT("Direct3D初期化失敗"), MB_OK);
		return;
	}
	// 各種3D描画設定を行う
	data->dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);	// カリング無効
	data->dev->SetRenderState(D3DRS_LIGHTING, TRUE);	// ライト有効
	data->dev->SetRenderState(D3DRS_ZENABLE, TRUE);		// Zバッファリング有効
	// ライトON
	data->light[0] = data->light[1] = data->light[2] = TRUE;

	// 実験①sample.xを読み込む。
	data->xdata = MyXData::load(&mydata, _TEXT("data/sample.x"));
	data->transz = 50;

	// アスペクト比を初期化.
	data->aspect = (float)WIDTH / (float)HEIGHT;

	ShowWindow(data->hWnd, SW_SHOWNORMAL);	// 作成したウィンドウを表示する。

	// イベントループ。
	// ブロック型関数GetMessageではなくノンブロック型関数のPeekMessageを使う。
	MSG msg;
	bool flag = 1;
	while (flag)
	{
		// メッセージがあるかどうか確認する
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			// メッセージがあるので処理する
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
			updateData(data);	// 位置の再計算
			drawData(data);		// 描画
		}
		// Sleepなど行っていないが、drawData関数内のdata->dev->Presentが
		// 画面更新待ちを行うので、結果的に1/60ごとにイベントループが回る。
	}
}

#define RELEASE(__xx__) if (__xx__) { __xx__->Release(); __xx__ = 0; }

// DataSetを解放する。
void ReleaseDataSet(DataSet* data)
{
	if (data->xdata) delete data->xdata;
	RELEASE(data->dev);
	RELEASE(data->pD3D);
}


// param argc コマンドラインから渡された引数の数
// param argv 引数の実体へのポインタ配列
// return 0 正常終了
int main(int argc, char* argv[])
{
	// このプログラムが実行されるときのインスタンスハンドルを取得
	mydata.hInstance = GetModuleHandle(NULL);
	MainLoop(&mydata);

	// 確保したリソースを解放
	ReleaseDataSet(&mydata);
	return 0;
}


// Direct3Dを初期化する
// param data データセット
// return 発生したエラーまたはD3D_OK
HRESULT initDirect3D(DataSet* data)
{
	HRESULT hr;

	// Direct3Dインスタンスオブジェクトを生成する。
	// D3D_SDK_VERSIONと、ランタイムバージョン番号が適切でないと、NULLが返る。
	data->pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	// NULLならランタイムが不適切
	if (data->pD3D == NULL) return E_FAIL;

	// PRESENTパラメータをゼロクリアし、適切に初期化
	ZeroMemory(&(data->d3dpp), sizeof(data->d3dpp));
	// ウィンドウモードに
	data->d3dpp.Windowed = TRUE;
	// Direct3Dにスワップエフェクトを任せる
	data->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// バックバッファはRGBそれぞれ８ビットで。
	data->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Present時に垂直同期に合わせる
	data->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// 3D用にZバッファ(depth buffer)を作成
	data->d3dpp.EnableAutoDepthStencil = TRUE;							// Direct3Dに深度バッファの管理を任せる
	data->d3dpp.AutoDepthStencilFormat = D3DFMT_D16;					// 深度バッファのフォーマット（通常はこの値で問題ない）

	// D3Dデバイスオブジェクトの作成。
	hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
								  D3DCREATE_HARDWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
	if (FAILED(hr))
	{
		// D3Dデバイスオブジェクトの作成。HAL&SOFT
		hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, data->hWnd,
									  D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
		if (FAILED(hr))
		{
			// D3Dデバイスオブジェクトの作成。REF&SOFT
			hr = data->pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, data->hWnd,
										  D3DCREATE_SOFTWARE_VERTEXPROCESSING, &(data->d3dpp), &(data->dev));
		}
	}
	if (FAILED(hr)) return hr;

	return D3D_OK;
}

// エラー発生時のHRESULTを文字列に変換するための補助関数
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
