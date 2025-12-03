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
	modelAttack_ = Model::CreateFromOBJ("attack", true);
	modelParticle_ = Model::CreateFromOBJ("particle", true);
	modelBlock_ = Model::CreateFromOBJ("cube", true);
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	modelDoor_ = Model::CreateFromOBJ("door", true);
	modelBackGround_ = Model::CreateFromOBJ("background", true);

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000.0f; // 遠くのオブジェクトまで描画するためにfarZを大きく設定

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	worldTransformBackGround1_.Initialize();
	worldTransformBackGround1_.translation_ = Vector3{ 10.0f, -6.0f, 20.0f };

	worldTransformBackGround2_.Initialize();
	worldTransformBackGround2_.translation_ = Vector3{ 40.0f, -6.0f, 20.0f };

	worldTransformBackGround3_.Initialize();
	worldTransformBackGround3_.translation_ = Vector3{ 70.0f, -6.0f, 20.0f };

	worldTransformBackGround4_.Initialize();
	worldTransformBackGround4_.translation_ = Vector3{ 100.0f, -6.0f, 20.0f };


	// 自キャラの生成
	player_ = new Player();
	// 座標を指定
	Vector3 playerPosition = { 5.0f * kBlockSize_, 0.0f, 5.0f * kBlockSize_ };
	// 自キャラの初期化
	player_->Initialize(model_, modelAttack_, &camera_, playerPosition);

	// 敵キャラの生成
	//3段目
	GenarateEnemies({ 24.0f * kBlockSize_, 0.0f, 8.0f * kBlockSize_ });
	GenarateEnemies({ 40.0f * kBlockSize_, 0.0f, 8.0f * kBlockSize_ });
	GenarateEnemies({ 36.0f * kBlockSize_, 0.0f, 4.0f * kBlockSize_ });

	// カメラコントローラーの初期化
	// 生成
	cameraController_ = new CameraController(camera_);
	// 初期化
	cameraController_->Initialize();
	// 追従対象をセット
	cameraController_->SetTarget(player_);
	// リセット（瞬間合わせ）
	cameraController_->Reset();

	// ゴールのドアを生成
	door_ = new Door();
	Vector3 doorPosition = { 8.0f * kBlockSize_, 0.0f, 8.0f * kBlockSize_ };
	door_->Initialize(modelDoor_, &camera_, doorPosition);

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

	UpdateWorldTransform(worldTransformBackGround1_);
	UpdateWorldTransform(worldTransformBackGround2_);
	UpdateWorldTransform(worldTransformBackGround3_);
	UpdateWorldTransform(worldTransformBackGround4_);

	// 自キャラの更新
	player_->Update();

	// 敵キャラの更新
	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

	// ゴールのドアの更新
	door_->Update();

	// 追従カメラの更新
	cameraController_->Update();
}

GameScene::~GameScene() {
	// フェード用オブジェクトの解放
	delete fade_;
	fade_ = nullptr;
	// 敵キャラの解放
	for (Enemy* enemy : enemies_) {
		// newで確保したメモリをdeleteで解放
		delete enemy;
	}
	// コンテナ自体を空にする
	enemies_.clear();
	// ブロックの解放
	for (auto& row : worldTransformBlocks_) {
		for (KamataEngine::WorldTransform* transform : row) {
			delete transform;
		}
		row.clear();
	}
	worldTransformBlocks_.clear();
	// 天球の解放
	delete skydome_;
	skydome_ = nullptr;
	// 自キャラの解放
	delete player_;
	player_ = nullptr;
	if (deathParticles_ != nullptr) {
		delete deathParticles_;
		deathParticles_ = nullptr;
	}
	// 追従カメラの解放
	delete cameraController_;
	delete door_;
	// デバッグカメラの解放
	delete debugCamera_;
	// 3Dモデルデータの解放
	delete model_;
	model_ = nullptr;
	delete modelAttack_;
	modelAttack_ = nullptr;
	delete modelParticle_;
	modelParticle_ = nullptr;
	delete modelBlock_;
	modelBlock_ = nullptr;
	delete modelSkydome_;
	modelSkydome_ = nullptr;
	delete modelEnemy_;
	modelEnemy_ = nullptr;
	delete modelDoor_;
	modelDoor_ = nullptr;
	delete modelBackGround_;
	modelBackGround_ = nullptr;
}

void GameScene::Update() {

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update(); // フェードの更新
		if (fade_->IsFinished()) { // フェードインが終了したら
			phase_ = Phase::kPlay;
		}
		//// 自キャラの更新
		//player_->Update();

		//// 敵キャラの更新
		//for (Enemy* enemy : enemies_) {
		//	enemy->Update();
		//}

		//// ゴールのドアの更新
		//door_->Update();

		//// 追従カメラの更新
		//cameraController_->Update();

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

		// ゴールのドアの更新
		door_->Update();

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
	case Phase::kClear: // 追加
		finished_ = true;
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

	// 背景の描画
	modelBackGround_->Draw(worldTransformBackGround1_, camera_);
	modelBackGround_->Draw(worldTransformBackGround2_, camera_);
	modelBackGround_->Draw(worldTransformBackGround3_, camera_);
	modelBackGround_->Draw(worldTransformBackGround4_, camera_);

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

	// ドアの描画
	door_->Draw();

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
		Sprite::PreDraw(dxCommon->GetCommandList());
		fade_->Draw();
		Sprite::PostDraw();
	}
}

void GameScene::GenarateBlocks() {
	// 床を作る
	const uint32_t kNumBlockHorizontal = 50; // 横方向（X軸）のブロック数
	const uint32_t kNumBlockVertical = 20; // 縦方向（Z軸）のブロック数

	// 配列のサイズを確保
	worldTransformBlocks_.resize(kNumBlockVertical);
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	}

	// 全ての WorldTransform* を nullptr で初期化（メモリリーク対策）
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			worldTransformBlocks_[i][j] = nullptr;
		}
	}

	// 必要な箇所にWorldTransformをnewして配置
	// 簡素な床を作成
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			// ここで、レベルデザインに合わせて特定の場所にブロックを配置するロジックを実装

			// X: j * kBlockSize, Y: -1.0f (床の高さ), Z: i * kBlockSize
			WorldTransform* worldTransform = new WorldTransform();
			worldTransform->Initialize();

			// 床を構成するブロックの位置
			worldTransform->translation_ = KamataEngine::Vector3{
				j * kBlockSize_ /*- (kNumBlockHorizontal * kBlockSize_ / 2.0f)*/, // X軸
				-1.0f, // Y軸（プレイヤーより下に配置）
				i * kBlockSize_ - (kNumBlockVertical * kBlockSize_ / 2.0f) // Z軸
			};
			worldTransform->scale_ = KamataEngine::Vector3{ kBlockSize_, 1.0f, kBlockSize_ }; // Y軸を薄くする

			worldTransformBlocks_[i][j] = worldTransform;
		}
	}

	// ジャンプ台、壁などは、上記のループ内で条件分岐を使って実装する
}

void GameScene::GenarateEnemies(const Vector3& position) {
	Enemy* newEnemy = new Enemy();
	// 敵キャラの初期化
	newEnemy->Initialize(modelEnemy_, &camera_, position);

	enemies_.push_back(newEnemy);
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

			// プレイヤー死亡時判定
			isPlayerDead_ = player_->IsDead();
		}
	}

#pragma endregion
#pragma region 自キャラとドアの当たり判定
	// ドアの座標
	aabb2 = door_->GetAABB();

	// 自キャラとドアの当たり判定
	if (IsCollision(aabb1, aabb2)) {
		// プレイヤーがドアに触れたら即座にクリアフェーズへ移行
		phase_ = Phase::kClear;
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
