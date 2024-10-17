/*
	3D_002.cpp

	プリミティブ

	教材「GP3_3D基礎v2.pptx」を参考にして input 部分に必要なコードを追加して
	プログラムを完成させてください。
*/
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <tchar.h>
#include <d3d9.h>			// Direct3D9 (DirectX9 Graphics)に必要
#include <d3dx9core.h>		// DirectX スプライトに必要

// ウィンドウの幅と高さを定数で設定する
#define WIDTH 1024			//< 幅
#define HEIGHT 768			//< 高さ

// ウィンドウタイトルバーに表示されるバージョン名
static const TCHAR version[] = _TEXT(__DATE__ " " __TIME__);

// ウィンドウクラス名.
static const TCHAR wndClsName[] = _TEXT("directx");

// クラス宣言。実体の定義より先にクラス名だけを宣言しておく。
struct DataSet;

// 関数宣言
HRESULT initDirect3D(DataSet* data);
const TCHAR* d3dErrStr(HRESULT res);
void ReleaseDataSet(DataSet* data);

// 頂点フォーマット構造体を定義する
// 頂点は、座標と法線ベクトル、ポイントサイズ、色情報を含む。
// これらのメンバ要素の並び順には決まりがあり、順番を間違えると正しい表示ができない。
struct MyVertex
{
	///+ input
	FLOAT nx, ny, nz;	// 法線ベクトル
	FLOAT pt;			// ポイントサイズ
	///+ input
	FLOAT tu, tv;		// テクスチャ座標
};

// 頂点構造体と一致するよう、D3DFVFを定義する。
// D3DFVF_XYZ ・・・頂点座標位置
// D3DFVF_NORMAL ・・・法線ベクトル
// D3DFVF_PSIZE ・・・頂点のポイントサイズ（点の見かけの大きさ）
// D3DFVF_DIFFUSE ・・・頂点の色
// D3DFVF_TEX1 ・・・テクスチャUV座標
#define kMYFVF (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_PSIZE|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// 頂点の位置は３Ｄ空間のワールド座標系（スクリーン座標系ではありません）
// 構造体MyVertexに従って、頂点を定義する。
// 後でインデックス指定するので、頂点配列での各頂点の並び順は適当で構わない。
static MyVertex vertices[] = {
	{ -10.0f, -10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff), 0.0f, 1.0f },	// 頂点0
	{ -10.0f,  10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0xff, 0x00, 0x00, 0xff), 0.0f, 0.0f },	// 頂点1
	{  10.0f, -10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0x00, 0xff, 0x00, 0xff), 1.0f, 1.0f },	// 頂点2
	{  10.0f,  10.0f, 0.0f,  0, 0, 1,  10.0f,  D3DCOLOR_RGBA(0x00, 0x00, 0xff, 0xff), 1.0f, 0.0f },	// 頂点3
};

// インデックスリストを作成する。この配列は三角形の頂点の組み合わせを
// 示すものである。数字は頂点配列でのインデックス位置を示し、３つずつで
// １つの三角形になる。
// ２つの三角形の頂点は以下のようになっている。
//  1      1 -- 3
//  | ＼     ＼ |
//  0 -- 2      2
// このため、インデックスリストは、(0, 1, 2), (1, 3, 2)となる。
// この数値並びは、頂点の順番が同じ向き(時計回り)になっていることに注意。
static short indices[] = {
	///+ input
};

// 線分を引くための頂点構造体。MyVertexと異なり、座標点とカラーのみを宣言。
struct MyLineVertex
{
	FLOAT x, y, z;
	DWORD color;
};

// MyLineVertex用のFVF
#define kXYZCOL (D3DFVF_XYZ|D3DFVF_DIFFUSE)

// 座標軸表示のためのデータ
static MyLineVertex axis[] = {
	{  30.0f, 0.0f, 0.0f,  D3DCOLOR_RGBA(0xff, 0x00, 0x00, 0xff), },
	{ -30.0f, 0.0f, 0.0f,  D3DCOLOR_RGBA(0xff, 0x00, 0x00, 0xff), },
	{ 0.0f,  30.0f, 0.0f,  D3DCOLOR_RGBA(0x00, 0xff, 0x00, 0xff), },
	{ 0.0f, -30.0f, 0.0f,  D3DCOLOR_RGBA(0x00, 0xff, 0x00, 0xff), },
	{ 0.0f, 0.0f,  30.0f,  D3DCOLOR_RGBA(0x00, 0x00, 0xff, 0xff), },
	{ 0.0f, 0.0f, -30.0f,  D3DCOLOR_RGBA(0x00, 0x00, 0xff, 0xff), },
};

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
	bool light[4];				//< ライトのON/OFF用フラグ
	// 各種フラグ
	bool flag[10];				//< 0:SOLID/WIRE

	IDirect3DVertexBuffer9* vb;	//< 頂点バッファ
	IDirect3DIndexBuffer9* ib;	//< インデックスバッファ
	IDirect3DTexture9* tex;		//< テクスチャ

	float cam_rotx;				//< カメラ回転量
	float cam_roty;
} mydata;

void resetData(DataSet* data)
{
	data->cam_rotx = data->cam_roty = 0;
	data->rotx = data->roty = 0;
}

////////////////////////// ::setCamera //////////////////////////
// カメラを設定する。カメラの位置、向きの他、スクリーン情報もある。
void setCamera(DataSet* data)
{
	HRESULT hr = D3D_OK;
	// ビュー行列作成のため、カメラ位置を決める
	// カメラは自分の位置、見ている点（注視点）、姿勢の３要素からなる。
	D3DXVECTOR3 cam(0, 0, data->transz);	// カメラ位置は (0, 0, transz)
	D3DXVECTOR3 camAt(0, 0, 0); // カメラの注視点は原点
	D3DXVECTOR3 camUp(0, 1, 0); // カメラは垂直姿勢 up=(0, 1, 0)

	// カメラ位置情報から行列を作成。RHは右手座標系の意味
	D3DXMATRIX camera;
	D3DXMatrixLookAtRH(&camera, &cam, &camAt, &camUp);

	// カメラを回転させる。SHIFTキー＋マウス左ボタン
	D3DXMATRIX mx, my;
	// X軸とY軸中心の回転を行列で表現
	D3DXMatrixRotationY(&my, data->cam_roty);
	D3DXMatrixRotationX(&mx, data->cam_rotx);
	// camera行列とかけ算。
	///+ input

	// ビュー行列として設定
	data->dev->SetTransform(D3DTS_VIEW, &camera);

	// 投影行列を設定する。スクリーンに投影するために必要。視野角はπ/2
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovRH(&proj, D3DX_PI / 2, data->aspect, 0.1f, 200.0f);
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

	// キーライトは強い白色に。
	light.Diffuse.r = 1.0;
	light.Diffuse.g = 1.0;
	light.Diffuse.b = 1.0;
	light.Ambient.r = 0.2f;
	light.Ambient.g = 0.2f;
	light.Ambient.b = 0.2f;
	// 方向は(1,-1,-1)を向くライト。
	light.Direction.x = 1;
	light.Direction.y = -1;
	light.Direction.z = -1;
	data->dev->SetLight(0, &light);		// 0番に設定
	data->dev->LightEnable(0, data->light[0]);	// 有効にする

	// フィルライトは弱い白色に。
	light.Diffuse.r = 0.6f;
	light.Diffuse.g = 0.6f;
	light.Diffuse.b = 0.6f;
	// 方向は(-1, 1,-1)を向くライト。０番ライトと左右に逆方向
	light.Direction.x = -1;
	light.Direction.y = 1;
	light.Direction.z = -1;
	data->dev->SetLight(1, &light);		// 1番に設定
	data->dev->LightEnable(1, data->light[1]);	// 有効にする

	// バックライトは強めに。
	light.Diffuse.r = 0.8f;
	light.Diffuse.g = 0.8f;
	light.Diffuse.b = 0.8f;
	// 方向は(-1, 1,1)を向くライト。モデルを後ろから照らす。
	light.Direction.x = -1;
	light.Direction.y = -1;
	light.Direction.z = 1;
	data->dev->SetLight(2, &light);		// 2番に設定
	data->dev->LightEnable(2, data->light[2]);	// 有効にする
}

////////////////////////// ::updateData //////////////////////////

// データをアップデートする。
// param data データセット
void updateData(DataSet* data)
{}


////////////////////////// ::drawData //////////////////////////
// DataSetに基づいて、スプライトを描画
// param data データセット
// return 発生したエラー
HRESULT drawData(DataSet* data)
{
	HRESULT hr = D3D_OK;

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

	// デフォルトはライティングOFF
	data->dev->SetRenderState(D3DRS_LIGHTING, data->light[3]);
	if (data->flag[0] == false)
	{
		data->dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	else
	{
		data->dev->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}

	// ワールド座標系の設定
	D3DXMATRIX ww;
	D3DXMatrixIdentity(&ww);		// まず単位行列にする
	D3DXMATRIX tmp;
	// Y軸中心の回転を行列で表現し、ww行列と積算
	D3DXMatrixRotationY(&tmp, data->roty);
	D3DXMatrixMultiply(&ww, &ww, &tmp);
	// X軸中心の回転を行列で表現し、ww行列と積算
	D3DXMatrixRotationX(&tmp, data->rotx);
	D3DXMatrixMultiply(&ww, &ww, &tmp);
	// ワールド座標として設定する。これ以降描画されるオブジェクトはこの行列の影響を受ける
	data->dev->SetTransform(D3DTS_WORLD, &ww);

	// FVFを指定
	data->dev->SetFVF(kMYFVF);
	// 頂点バッファ、インデックスバッファをストリームにセット
	///+ input
	///+ input
	// テクスチャの指定をOFFに
	if (data->flag[1])
	{
		data->dev->SetTexture(0, data->tex);
	}
	else
	{
		data->dev->SetTexture(0, NULL);
	}
	// トライアングル・リストではPSIZEは無視される
	///+ input

	// これ以降はライティングOFF、テクスチャOFFで
	data->dev->SetRenderState(D3DRS_LIGHTING, FALSE);
	data->dev->SetTexture(0, NULL);

	// 頂点を描画(D3DPT_POINTLIST).
	// DIFFUSEが邪魔なので、FVFを変更。構造体の並びに矛盾がなければFVFを
	// 変更できる。つまり、構造体の途中のメンバを外すことはできないが、
	// 最後のメンバであれば無視することが可能。
	// DIFFUSEが有効のままだと、頂点カラーが入ってしまう。
	data->dev->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_PSIZE);
	data->dev->DrawPrimitive(D3DPT_POINTLIST, 0, 4);

	// 座標軸を描画(D3DPT_LINELIST)
	D3DXMatrixIdentity(&ww);
	data->dev->SetTransform(D3DTS_WORLD, &ww);
	// FVFを変更。頂点バッファの構造に合わせる。
	data->dev->SetFVF(kXYZCOL);
	data->dev->DrawPrimitiveUP(D3DPT_LINELIST, 3, (VOID*)axis, sizeof(MyLineVertex));

	data->dev->EndScene();

	// 実際に画面に表示。バックバッファからフロントバッファへの転送
	// デバイス生成時のフラグ指定により、ここでVSYNCを待つ。
	data->dev->Present(NULL, NULL, NULL, NULL);

	// Direct3D表示後、GDIにより文字列表示
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
	TextOut(hdc, text_posx, text_posy, text, (int)_tcsclen(text));// 文字列の表示
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

	RECT rc = { 0, 0, WIDTH, HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	// ウィンドウを作成する。クラス名は"directx"
	data->hWnd = CreateWindow(wndClsName, version, WS_OVERLAPPEDWINDOW,
							  0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, data->hInstance, NULL);

	return data->hWnd;
}

// テクスチャfnameを読み込む。エラーが発生したら例外を発生する。
// param fname ファイル名。const修飾子をつけ関数内で値を変更しないことを宣言する。
// param tex 作成するテクスチャへのポインタを入れるためのポインタ。
HRESULT loadTexture(IDirect3DDevice9* dev, const TCHAR fname[], IDirect3DTexture9** tex)
{
	HRESULT hr = D3DXCreateTextureFromFile(dev, fname, tex);
	if (FAILED(hr))
	{
		MessageBox(NULL, _TEXT("テクスチャ読み込み失敗"), fname, MB_OK);
		throw hr;		// エラーが発生したので例外を投げる.
	}
	return hr;
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
	data->dev->SetRenderState(D3DRS_ZENABLE, TRUE);		// Zバッファリング有効
	// ライトON
	data->light[0] = data->light[1] = data->light[2] = TRUE;
	data->aspect = (float)WIDTH / (float)HEIGHT;
	data->rotx = data->roty = 0.0f;
	data->transz = 50.0f;

	// 頂点バッファを作成。sizeof(vertices)バイト分のメモリを確保する。
	// バッファはビデオメモリに作られるため、高速な描画が期待できる。
	hr = data->dev->CreateVertexBuffer(sizeof(vertices), 0, kMYFVF,
									   D3DPOOL_DEFAULT, &(data->vb), NULL);

	// 頂点バッファをロックして、ポインタアドレスを取得する。
	// ロックするとメインメモリにデータが配置される。
	// その後頂点配列(vertices)をコピーし、直ちにロック解除する。
	// ロック解除操作により、VRAMにデータが転送される。
	VOID* pV;
	///+ input
	///+ input
	///+ input

	// インデックスバッファの作成とデータコピー
	hr = data->dev->CreateIndexBuffer(sizeof(indices), 0, D3DFMT_INDEX16,
									  D3DPOOL_DEFAULT, &(data->ib), NULL);
	VOID* pI;
	hr = data->ib->Lock(0, sizeof(indices), &pI, 0);
	CopyMemory(pI, indices, sizeof(indices));
	data->ib->Unlock();

	// テクスチャ画像を読み込む
	loadTexture(data->dev, _TEXT("data/koara.jpg"), &(data->tex));

	ShowWindow(data->hWnd, SW_SHOWNORMAL);	// 作成したウィンドウを表示する。

	// 反時計回りの頂点を持つ背面をカリング
	//data->dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	// カリングを無効に
	data->dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

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
	RELEASE(data->ib);
	RELEASE(data->vb);
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
	data->d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// バックバッファはRGBそれぞれ８ビットで。
	data->d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	// Present時に垂直同期に合わせる
	data->d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// 3D用にZバッファ(depth buffer)を作成
	data->d3dpp.EnableAutoDepthStencil = TRUE;
	data->d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

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
