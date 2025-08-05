#include "GameScene.h"
#include "MatrixGenerators.h"
#include "EngineMathFunctions.h"
#include "WorldTransform.h"

using namespace KamataEngine;

void GameScene::Initialize() {
	// ゲームプレイフェーズから開始
	phase_ = Phase::kPlay;

	// ファイル名を指定してテクスチャを読み込む

	// 3Dモデルデータの生成
	model_ = Model::CreateFromOBJ("player", true);
	modelParticle_ = Model::CreateFromOBJ("particle", true);
	modelBlock_ = Model::CreateFromOBJ("cube", true);
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000.0f; // 遠くのオブジェクトまで描画するためにfarZを大きく設定

	// マップチップフィールドの生成
	mapChipField_ = new MapChipField;
	// マップチップフィールドの初期化
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	// 自キャラの生成
	player_ = new Player();
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(4, 18);
	// 自キャラの初期化
	player_->Initialize(model_, &camera_, playerPosition);

	// 敵キャラの生成
	for (int32_t i = 0; i < kEnemyNum; ++i) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(18, 18 - i);
		// 敵キャラの初期化（座標をマップチップ番号で指定）
		newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);

		enemies_.push_back(newEnemy);
	}

	// マップチップフィールドの参照をセット
	player_->SetMapChipField(mapChipField_);

	// カメラコントローラーの初期化
	// 生成
	cameraController_ = new CameraController(camera_);
	// 初期化
	cameraController_->Initialize();
	// 追従対象をセット
	cameraController_->SetTarget(player_);
	// リセット（瞬間合わせ）
	cameraController_->Reset();

	// ブロックの生成
	GenarateBlocks();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(kWindowWidth, kWindowHeight);

	// フェード用オブジェクトの生成と初期化
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeTime); // フェードインから始める

	// 初期フェーズはフェードイン
	phase_ = Phase::kFadeIn;
}

GameScene::~GameScene() {
	// 3Dモデルデータの解放
	delete model_;
	delete modelParticle_;
	delete modelBlock_;
	delete modelSkydome_;
	delete modelEnemy_;
	// 自キャラの解放
	delete player_;
	delete deathParticles_;
	// 敵キャラの解放
	for (Enemy* enemy : enemies_) {
		// newで確保したメモリをdeleteで解放
		delete enemy;
	}
	// コンテナ自体を空にする
	enemies_.clear();

	// マップチップフィールドの解放
	delete mapChipField_;
	// 天球の解放
	delete skydome_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	// デバッグカメラの解放
	delete debugCamera_;

	delete fade_;
}

void GameScene::Update() {

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update(); // フェードの更新
		if (fade_->IsFinished()) { // フェードインが終了したら
			phase_ = Phase::kPlay;
		}
		break;
	case Phase::kPlay:
		// 天球の更新
		skydome_->Update();

		// 自キャラの更新
		player_->Update();

		// 敵キャラの更新
for (Enemy* enemy : enemies_) {
	enemy->Update();
}

// 追従カメラの更新
cameraController_->Update();

#ifdef _DEBUG

if (Input::GetInstance()->TriggerKey(DIK_TAB)) {
	// デバッグカメラ有効フラグをトグル
	if (isDebugCameraActive_) {
		isDebugCameraActive_ = false;
	} else {
		isDebugCameraActive_ = true;
	}
}
#endif

// カメラの処理
if (isDebugCameraActive_) {
	// デバッグカメラの更新
	debugCamera_->Update();
	camera_.matView = debugCamera_->GetCamera().matView;
	camera_.matProjection = debugCamera_->GetCamera().matProjection;
	// ビュープロジェクション行列の転送
	camera_.TransferMatrix();
} else {
	// ビュープロジェクション行列の更新と転送
	camera_.UpdateMatrix();
}

// ブロックの更新
for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
	for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
		if (!worldTransformBlock)
			continue;

		// 定数バッファに転送する
		UpdateWorldTransform(*worldTransformBlock);
	}
}

// 全ての当たり判定を行う
CheckAllCollisions();

ChangePhase();

break;
	case Phase::kDeath:
		// 天球の更新
		skydome_->Update();

		// 敵キャラの更新
		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		// デスパーティクルの更新
		if (deathParticles_ != nullptr) {
			deathParticles_->Update();
		}

#ifdef _DEBUG

		if (Input::GetInstance()->TriggerKey(DIK_TAB)) {
			// デバッグカメラ有効フラグをトグル
			if (isDebugCameraActive_) {
				isDebugCameraActive_ = false;
			} else {
				isDebugCameraActive_ = true;
			}
		}
#endif

		// カメラの処理
		if (isDebugCameraActive_) {
			// デバッグカメラの更新
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
				if (!worldTransformBlock)
					continue;

				// 定数バッファに転送する
				UpdateWorldTransform(*worldTransformBlock);
			}
		}

		if (deathParticles_ && deathParticles_->IsFinished()) {
			// 死亡演出が終わったらフェードアウト開始
			fade_->Start(Fade::Status::FadeOut, kFadeTime);
			phase_ = Phase::kFadeOut;
		}

		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) { // フェードアウトが終了したら
			finished_ = true;
		}
		break;
	}
}

void GameScene::Draw() {
	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// 天球の描画
	skydome_->Draw();

	// 自キャラの描画
	if (!player_->IsDead()) {
		player_->Draw();
	}

	if (deathParticles_ != nullptr) {
		deathParticles_->Draw();
	}

	// 敵キャラの描画
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;

			// 3Dモデルの描画
			modelBlock_->Draw(*worldTransformBlock, camera_);
		}
	}

	// 3Dモデル描画後処理
	Model::PostDraw();

	// フェードイン中/フェードアウト中はフェードの描画を行う
	if (phase_ == Phase::kFadeIn || phase_ == Phase::kFadeOut) {
		fade_->Draw();
	}
}

void GameScene::GenarateBlocks() {

	// 要素数
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定（縦方向のブロック数）
	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		// 1列の要素数を設定（横方向のブロック数）
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}

}

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと敵キャラの当たり判定
	// 判定対象1と2の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player_->GetAABB();

	// 自キャラと敵弾全ての当たり判定
	for (Enemy* enemy : enemies_) {
		// 敵弾の座標
		aabb2 = enemy->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 自キャラの衝突時関数を呼び出す
			player_->OnCollision(enemy);
			// 敵の衝突時関数を呼び出す
			enemy->OnCollision(player_);
		}
	}

#pragma endregion
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		if (player_->IsDead()) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			// 生成処理
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelParticle_, &camera_, deathParticlesPosition);

		}
		break;
	case Phase::kDeath:

		break;
	}
}
