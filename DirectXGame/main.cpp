#include "GameClearScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "StageSelectScene.h"
#include "TitleScene.h"
#include <Windows.h>

using namespace KamataEngine;

// シーン(型)
enum class Scene {

	kUnknown = 0,

	kTitle,
	kStageSelect,
	kGame,
	kClear,
};

// 現在シーン(型)
Scene scene = Scene::kUnknown;

GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
StageSelectScene* stageSelectScene = nullptr;
GameClearScene* gameClearScene = nullptr;

void ChangeScene();

void UpdateScene();

void DrawScene();

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2B_01_アカミネ_レン_ウツリハガネ");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 最初のシーンの初期化
#ifdef _DEBUG
	/*scene = Scene::kGame;
	gameScene = new GameScene;
	gameScene->Initialize(1);*/
	scene = Scene::kStageSelect;
	stageSelectScene = new StageSelectScene;
	stageSelectScene->Initialize();
#else
	scene = Scene::kTitle;
	titleScene = new TitleScene;
	titleScene->Initialize();
#endif // _DEBUG

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// シーン切り替え
		ChangeScene();
		// 現在シーン更新
		UpdateScene();

		// 描画開始
		dxCommon->PreDraw();

		// 現在シーンの描画
		DrawScene();

		// 描画終了
		dxCommon->PostDraw();
	}
	// 終了時、シーンオブジェクトが残っていたら解放する
	// タイトルシーンの解放
	if (titleScene) {
		delete titleScene;
		titleScene = nullptr;
	}
	if (stageSelectScene) {
		delete stageSelectScene;
		stageSelectScene = nullptr;
	}
	// ゲームシーンの解放
	if (gameScene) {
		delete gameScene;
		gameScene = nullptr;
	}
	// ゲームクリアシーンの解放
	if (gameClearScene) {
		delete gameClearScene;
		gameClearScene = nullptr;
	}

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}

void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kStageSelect;
			// 旧シーンの解放
			delete titleScene;
			titleScene = nullptr;
			// 新シーンの生成と初期化
			stageSelectScene = new StageSelectScene;
			stageSelectScene->Initialize();
		}
		break;

	case Scene::kStageSelect:
		if (stageSelectScene->IsFinished()) {
			// 選択されたステージ番号を取得
			int stageNo = stageSelectScene->GetSelectedStageNo();

			// ステージ選択 -> ゲームへ
			scene = Scene::kGame;
			delete stageSelectScene;
			stageSelectScene = nullptr;

			gameScene = new GameScene;
			gameScene->Initialize(stageNo); // 選んだステージ番号を渡す
		}
		break;

	case Scene::kGame:
		if (gameScene->IsFinished()) {
			if (gameScene->IsPlayerDead()) {
				// シーン変更
				scene = Scene::kStageSelect;
				// 旧シーンの解放
				delete gameScene;
				gameScene = nullptr;
				// 新シーンの生成と初期化
				stageSelectScene = new StageSelectScene;
				stageSelectScene->Initialize();
			} else {
				// シーン変更
				scene = Scene::kClear;
				// 旧シーンの解放
				delete gameScene;
				gameScene = nullptr;
				// 新シーンの生成と初期化
				gameClearScene = new GameClearScene;
				gameClearScene->Initialize();
			}
		}
		break;
	case Scene::kClear:
		if (gameClearScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;
			// 旧シーンの解放
			delete gameClearScene;
			gameClearScene = nullptr;
			// 新シーンの生成と初期化
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kStageSelect:
		if (stageSelectScene) {
			stageSelectScene->Update();
		}
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	case Scene::kClear:
		gameClearScene->Update();
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) {
			titleScene->Draw();
		}
		break;
	case Scene::kStageSelect:
		if (stageSelectScene) {
			stageSelectScene->Draw();
		}
		break;
	case Scene::kGame:
		if (gameScene) {
			gameScene->Draw();
		}
		break;
	case Scene::kClear:
		if (gameClearScene) {
			gameClearScene->Draw();
		}
		break;
	}
}
