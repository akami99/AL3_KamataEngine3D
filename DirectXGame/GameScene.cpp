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
	//modelBackGround_ = Model::CreateFromOBJ("background", true);

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000.0f; // 遠くのオブジェクトまで描画するためにfarZを大きく設定

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	/*worldTransformBackGround1_.Initialize();
	worldTransformBackGround1_.translation_ = Vector3{ 10.0f, -6.0f, 20.0f };

	worldTransformBackGround2_.Initialize();
	worldTransformBackGround2_.translation_ = Vector3{ 40.0f, -6.0f, 20.0f };

	worldTransformBackGround3_.Initialize();
	worldTransformBackGround3_.translation_ = Vector3{ 70.0f, -6.0f, 20.0f };

	worldTransformBackGround4_.Initialize();
	worldTransformBackGround4_.translation_ = Vector3{ 100.0f, -6.0f, 20.0f };*/


	// 自キャラの生成
	player_ = new Player();
	// 座標を指定
	Vector3 playerPosition = { 10.0f * kBlockSize_, 1.0f, 3.0f * kBlockSize_ };
	// 自キャラの初期化
	player_->Initialize(model_, modelAttack_, &camera_, playerPosition);

	// 敵キャラの生成(奥側から)
	GenarateEnemies({ 14.0f * kBlockSize_, 1.0f, 18.0f * kBlockSize_ });
	GenarateEnemies({ 16.0f * kBlockSize_, 1.0f, 18.0f * kBlockSize_ });
	GenarateEnemies({ 6.0f * kBlockSize_, 1.0f, 17.0f * kBlockSize_ });
	GenarateEnemies({ 10.0f * kBlockSize_, 1.0f, 14.0f * kBlockSize_ });
	GenarateEnemies({ 13.0f * kBlockSize_, 1.0f, 12.0f * kBlockSize_ });
	GenarateEnemies({ 5.0f * kBlockSize_, 1.0f, 10.0f * kBlockSize_ });
	GenarateEnemies({ 7.0f * kBlockSize_, 1.0f, 10.0f * kBlockSize_ });
	GenarateEnemies({ 17.0f * kBlockSize_, 1.0f, 6.0f * kBlockSize_ });

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
	Vector3 doorPosition = { 19.0f * kBlockSize_, 1.0f, 19.0f * kBlockSize_ };
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

	/*UpdateWorldTransform(worldTransformBackGround1_);
	UpdateWorldTransform(worldTransformBackGround2_);
	UpdateWorldTransform(worldTransformBackGround3_);
	UpdateWorldTransform(worldTransformBackGround4_);*/

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
	// 壁・障害物の解放
	for (WorldTransform* block : collidableBlocks_) {
		delete block;
	}
	collidableBlocks_.clear();

	// 床の解放
	delete floorTransform_;
	floorTransform_ = nullptr;
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
	/*delete modelBackGround_;
	modelBackGround_ = nullptr;*/
}

void GameScene::Update() {

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update(); // フェードの更新
		if (fade_->IsFinished()) { // フェードインが終了したら
			phase_ = Phase::kPlay;
		}

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
		UpdateWorldTransform(*floorTransform_);
		for (WorldTransform* block : collidableBlocks_) {
			// 定数バッファに転送する
			UpdateWorldTransform(*block);
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
		// ブロックの更新
		UpdateWorldTransform(*floorTransform_);
		for (WorldTransform* block : collidableBlocks_) {
			// 定数バッファに転送する
			UpdateWorldTransform(*block);
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
		UpdateWorldTransform(*floorTransform_);
		for (WorldTransform* block : collidableBlocks_) {
			// 定数バッファに転送する
			UpdateWorldTransform(*block);
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
	/*modelBackGround_->Draw(worldTransformBackGround1_, camera_);
	modelBackGround_->Draw(worldTransformBackGround2_, camera_);
	modelBackGround_->Draw(worldTransformBackGround3_, camera_);
	modelBackGround_->Draw(worldTransformBackGround4_, camera_);*/

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
	modelBlock_->Draw(*floorTransform_, camera_);
	for (WorldTransform* block : collidableBlocks_) {
		modelBlock_->Draw(*block, camera_);
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
	// 床を生成

	// 床のサイズ
	const float kAreaSize = 40.0f;
	const float kpositionOfset = kAreaSize * 0.5f;
	// 床のWorldTransformを生成
	floorTransform_ = new WorldTransform();
	floorTransform_->Initialize();
	// kAreaSize x kAreaSize の広さ
	floorTransform_->scale_ = Vector3{ kAreaSize, 1.0f, kAreaSize };
	// 位置を設定
	floorTransform_->translation_ = Vector3{ kpositionOfset, 0.0f, kpositionOfset };

	// 衝突判定を行う壁や障害物の生成 (collidableBlocks_ に格納)

	// 壁や障害物を作成するヘルパー関数
	auto createWall = [&](const Vector3& pos, const Vector3& scale) {
		WorldTransform* worldTransform = new WorldTransform();
		worldTransform->Initialize();
		worldTransform->translation_ = pos;
		worldTransform->scale_ = scale;
		collidableBlocks_.push_back(worldTransform); // リストに格納
		};

	// レベルデザインに合わせて壁を配置
	const float kOutWallY = 1.5f; // 地上高
	const float kOutWallThickness = 1.0f; // 外壁の厚さ
	const float kOutWallHalfThickness = kOutWallThickness * 0.5f;
	const float kOutWallLength = kAreaSize; // 外壁の長さ
	const float kOutWallHeight = 2.0f; // 外壁の高さ

	const float kWallY = 1.5f; // 地上高
	const float kWallThickness = 2.0f; // 壁の厚さ
	const float kWallLength = 20.0f; // 壁の長さ
	const float kWallHeight = 2.0f; // 壁の高さ

	// 四方の外壁
	
	// 北側の外壁
	createWall(
		KamataEngine::Vector3{ kpositionOfset + kOutWallHalfThickness, kOutWallY, kOutWallLength + kOutWallHalfThickness },
		KamataEngine::Vector3{ kOutWallLength + kOutWallThickness, kOutWallHeight, kOutWallThickness }
	);

	// 南側の外壁
	createWall(
		KamataEngine::Vector3{ kpositionOfset - kOutWallHalfThickness, kOutWallY, -kOutWallHalfThickness },
		KamataEngine::Vector3{ kOutWallLength + kOutWallThickness, kOutWallHeight, kOutWallThickness }
	);

	// 東側の外壁
	createWall(
		KamataEngine::Vector3{ kOutWallLength + kOutWallHalfThickness, kOutWallY, kpositionOfset - kOutWallHalfThickness },
		KamataEngine::Vector3{ kOutWallThickness, kOutWallHeight, kOutWallLength + kOutWallThickness }
	);

	// 西側の外壁
	createWall(
		KamataEngine::Vector3{ -kOutWallHalfThickness, kOutWallY, kpositionOfset + kOutWallHalfThickness },
		KamataEngine::Vector3{ kOutWallThickness, kOutWallHeight, kOutWallLength + kOutWallThickness }
	);

	// 北側の壁 (Z = +10.0f のライン)
	createWall(
		KamataEngine::Vector3{ kpositionOfset + kBlockSize_ * 5.0f, kWallY, kpositionOfset + kBlockSize_ * 5.0f },
		KamataEngine::Vector3{ kWallLength, kWallHeight, kWallThickness }
	);

	// 南側の壁 (Z = -10.0f のライン)
	createWall(
		KamataEngine::Vector3{ kpositionOfset - kBlockSize_ * 5.0f, kWallY, kpositionOfset - kBlockSize_ * 5.0f },
		KamataEngine::Vector3{ kWallLength, kWallHeight, kWallThickness }
	);

	// 障害物 (中央付近)
	createWall(
		KamataEngine::Vector3{ kpositionOfset - kBlockSize_ * 3.0f, kWallY, kpositionOfset + kBlockSize_ * 2.0f },
		KamataEngine::Vector3{ kBlockSize_ * 2.0f, kBlockSize_, kBlockSize_ * 2.0f }
	);
}

void GameScene::GenarateEnemies(const Vector3& position) {
	Enemy* newEnemy = new Enemy();
	// 敵キャラの初期化
	newEnemy->Initialize(modelEnemy_, &camera_, position);

	enemies_.push_back(newEnemy);
}

void GameScene::CheckAllCollisions() {
#pragma region 自キャラの攻撃と敵の当たり判定
	// プレイヤーが「攻撃中」かつ「突進フェーズ」の場合のみ判定
	if (player_->GetBehavior() == Player::Behavior::kAttack &&
		player_->GetAttackPhase() == Player::AttackPhase::Rush) {

		// プレイヤーの攻撃範囲を取得
		AABB attackBox = player_->GetAttackAABB();

		for (Enemy *enemy : enemies_) {
			// すでに死んでいる敵はスキップ
			if (enemy->IsDead()) continue;

			// 敵の当たり判定を取得
			AABB enemyBox = enemy->GetAABB();

			// GameSceneにある既存の衝突判定関数を利用
			if (IsCollision(attackBox, enemyBox)) {
				// 当たったら敵を撃破状態にする
				enemy->OnSlay();

				// (アドバイス) エフェクトなどを出すならここ
				//deathParticles_->Emit(enemy->GetTranslation()); 
			}
		}
	}

	// --- 倒れた敵のクリーンアップ (アドバイスの内容) ---
	// デスフラグが立っている敵をリストから削除し、メモリを解放する
	enemies_.remove_if([](Enemy *enemy) {
		if (enemy->IsDead()) {
			delete enemy; // メモリ解放
			return true;  // リストから削除
		}
		return false;
		});

#pragma endregion
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
#pragma region 自キャラとブロックの当たり判定
	// 判定対象1と2の座標
	AABB playerAABB, blockAABB;

	// プレイヤーのAABBと現在の速度を取得
	playerAABB = player_->GetAABB();
	Vector3 playerVelocity = player_->GetVelocity();

	// 全ての衝突判定ブロックと自キャラの当たり判定
	for (WorldTransform* block : collidableBlocks_) {
		// ブロックのAABBを取得
		blockAABB = GetAABB(*block);

		// AABB同士の交差判定
		if (IsCollision(playerAABB, blockAABB)) {

			// 衝突を解決し、プレイヤーの移動量を修正

			// 1. 衝突を解消するために必要な最小移動ベクトル(MTV)を取得
			Vector3 resolveVector = CalculateAABBOverlap(playerAABB, blockAABB);

			// 2. プレイヤーの座標をMTVの分だけ移動させて、めり込みを解除する
			// resolveVectorには、XかZのどちらか一方にのみ、押し戻し量が入っている（MTV）
			Vector3 pushBackVector = player_->GetTranslation();
			pushBackVector += resolveVector;
			player_->SetTranslation(pushBackVector);

			// 3. 衝突が発生した軸方向の速度をリセット（壁にぶつかったら止まる）
			if (std::abs(resolveVector.x) > 0.0f) {
				// X軸方向のめり込みを解消した場合
				playerVelocity.x = 0.0f;
			}
			if (std::abs(resolveVector.z) > 0.0f) {
				// Z軸方向のめり込みを解消した場合
				playerVelocity.z = 0.0f;
			}
		}
	}

	// 衝突解決後の速度をプレイヤーにフィードバック
	player_->SetVelocity(playerVelocity);
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
