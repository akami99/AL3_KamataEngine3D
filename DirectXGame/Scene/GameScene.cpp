#include "GameScene.h"
#include "Enemy/EnemyBullet.h"
#include "EngineMathFunctions.h"
#include "MatrixGenerators.h"
#include "WorldTransform.h"

using namespace KamataEngine;

void GameScene::Initialize(int stageNo) {
	// ゲームプレイフェーズから開始
	phase_ = Phase::kPlay;

	// ステージ番号を記録
	stageNo_ = stageNo;

	// ファイル名を指定してテクスチャを読み込む
	tutorialButtonHandle_ = TextureManager::Load("./Resources/tutorialButton/tutorialButton.png");

	// 3Dモデルデータの生成
	model_ = Model::CreateFromOBJ("player", true);
	modelAttack_ = Model::CreateFromOBJ("attack", true);
	modelParticle_ = Model::CreateFromOBJ("particle", true);
	modelBlock_ = Model::CreateFromOBJ("cube", true);
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);
	modelEnemyBullet_ = modelParticle_; // 敵の弾はパーティクルモデルを使い回す
	modelDoor_ = Model::CreateFromOBJ("door", true);

	spriteTutorialButton_ = Sprite::Create(tutorialButtonHandle_, { 15.0f, 625.0f }, {1.0f, 1.0f, 1.0f, 0.8f});

	// カメラの初期化
	camera_.Initialize();
	camera_.farZ = 1000.0f; // 遠くのオブジェクトまで描画するためにfarZを大きく設定

	// 天球の生成
	skydome_ = new Skydome();
	// 天球の初期化
	skydome_->Initialize(modelSkydome_, &camera_);

	// カメラコントローラーの初期化
	// 生成
	cameraController_ = new CameraController(camera_);
	// 初期化
	cameraController_->Initialize();

	// ステージの生成と初期化
	LoadStage();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(kWindowWidth, kWindowHeight);

	// フェード用オブジェクトの生成と初期化
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, kFadeTime); // フェードインから始める

	// 初期フェーズはフェードイン
	phase_ = Phase::kFadeIn;

	//  ステージの更新
	stage_->Update();

	// 追従カメラの更新
	cameraController_->Update();

	// 操作方法表示
	tutorial_ = new Tutorial();
	tutorial_->Initialize();

	// ページ（モデル）を追加
	// Resourcesフォルダに "tutorial_page1" などのOBJフォルダを用意してください
	tutorial_->AddPage("tutorial_move");
	tutorial_->AddPage("tutorial_dush");
	tutorial_->AddPage("tutorial_attack");
}

GameScene::~GameScene() {
	delete spriteTutorialButton_;
	spriteTutorialButton_ = nullptr;
	// フェード用オブジェクトの解放
	delete fade_;
	fade_ = nullptr;
	// 天球の解放
	delete skydome_;
	skydome_ = nullptr;
	if (deathParticles_ != nullptr) {
		delete deathParticles_;
		deathParticles_ = nullptr;
	}
	// ステージの解放
	delete stage_;
	stage_ = nullptr;
	// チュートリアルを解放
	delete tutorial_;
	tutorial_ = nullptr;
	// 追従カメラの解放
	delete cameraController_;
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
}

void GameScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();           // フェードの更新
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
		break;
	case Phase::kPlay:
		// チュートリアル中はゲームの更新を止める場合
		if (tutorial_->IsActive()) {
			tutorial_->Update(camera_);
			return; // ここでreturnするとゲーム本編が止まる
		}
		// チュートリアル開始
		if (Input::GetInstance()->TriggerKey(DIK_T)) {
			// 開始
			tutorial_->Open();
		}

		// 天球の更新
		skydome_->Update();

		// ステージの更新
		stage_->Update();

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

		// 全ての当たり判定を行う
		CheckAllCollisions();

		ChangePhase();

		break;
	case Phase::kDeath:
		// 天球の更新
		skydome_->Update();

		// ステージの更新
		stage_->Update();

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

		// 敵の弾の衝突判定
		CheckEnemyBullets();

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
	case Phase::kClear:
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

	if (deathParticles_ != nullptr) {
		deathParticles_->Draw();
	}

	// ステージの描画
	stage_->Draw(camera_);


	// 最前面に描画
	tutorial_->Draw(camera_);

	// 3Dモデル描画後処理
	Model::PostDraw();

	Sprite::PreDraw(dxCommon->GetCommandList());

	// 操作説明を表示するキー表示
	spriteTutorialButton_->Draw();

	// フェードイン中/フェードアウト中はフェードの描画を行う
	if (phase_ == Phase::kFadeIn || phase_ == Phase::kFadeOut) {
		fade_->Draw();
	}

	Sprite::PostDraw();
}

void GameScene::CheckAllCollisions() {
	CheckPlayerAndEnemies();
	CheckEnemyBullets();
	CheckPlayerAttack();
	CheckPlayerAndDoor();
	CheckPlayerAndBlocks();
}

void GameScene::CheckPlayerAndEnemies() {
	// 自キャラの座標
	Player* player = stage_->GetPlayer();
	AABB playerAABB = player->GetAABB();

	// 自キャラと敵全ての当たり判定
	for (Enemy* enemy : stage_->GetEnemies()) {
		// 敵の座標
		AABB enemyAABB = enemy->GetAABB();

		// AABB同士の交差判定
		if (IsCollision(playerAABB, enemyAABB)) {
			// 自キャラの衝突時関数を呼び出す
			player->OnCollision(enemy);
			// 敵の衝突時関数を呼び出す
			enemy->OnCollision(player);

			// プレイヤー死亡時判定
			isPlayerDead_ = player->IsDead();
		}
	}
}

void GameScene::CheckEnemyBullets() {
	for (Enemy* enemy : stage_->GetEnemies()) {
		// 敵が発射した全ての弾を取得
		const std::list<EnemyBullet*>& bullets = enemy->GetBullets();
		// 各弾に対して当たり判定をチェック
		for (EnemyBullet* bullet : bullets) {
			if (bullet->IsDead()) {
				// 死んでいる弾はスキップ
				continue;
			}
			// 弾の座標
			Vector3 bulletPos = bullet->GetWorldPosition();
			float r = EnemyBullet::kRadius; // 弾の半径
			// 弾のAABB
			AABB bulletAABB;
			bulletAABB.min = Vector3{bulletPos.x - r, bulletPos.y - r, bulletPos.z - r};
			bulletAABB.max = Vector3{bulletPos.x + r, bulletPos.y + r, bulletPos.z + r};
			// --- プレイヤーとの衝突判定 ---
			Player* player = stage_->GetPlayer();
			if (player && !player->IsDead()) {
				// 自キャラのAABBを取得
				AABB playerAABB = player->GetAABB();
				// AABB同士の交差判定
				if (IsCollision(bulletAABB, playerAABB)) {
					// 弾の衝突時関数を呼び出す
					bullet->OnCollision();
					// 自キャラの衝突時関数を呼び出す
					player->OnCollision(bullet);
					// プレイヤー死亡時判定
					isPlayerDead_ = player->IsDead();
				}
			}

			// --- 壁（ブロック）との衝突判定 ---
			for (WorldTransform* block : stage_->GetBlocks()) {
				// ブロックのAABBを取得
				AABB blockAABB = GetAABB(*block);

				if (IsCollision(bulletAABB, blockAABB)) {
					// 壁に当たったら弾だけ消す
					bullet->OnCollision();
					break; // この弾はもう消えたので、他のブロックとの判定は不要
				}
			}
		}
	}
}

void GameScene::CheckPlayerAttack() {
	Player* player = stage_->GetPlayer();
	// プレイヤーが「攻撃中」かつ「突進フェーズ」の場合のみ判定
	if (player->GetBehavior() == Player::Behavior::kAttack && player->GetAttackPhase() == Player::AttackPhase::Rush) {

		// プレイヤーの攻撃範囲を取得
		AABB attackBox = player->GetAttackAABB();

		for (Enemy* enemy : stage_->GetEnemies()) {
			// すでに死んでいる敵はスキップ
			if (enemy->IsDead())
				continue;

			// 敵の当たり判定を取得
			AABB enemyBox = enemy->GetAABB();

			// GameSceneにある既存の衝突判定関数を利用
			if (IsCollision(attackBox, enemyBox)) {
				// 当たったら敵を撃破状態にする
				enemy->OnSlay();

				// エフェクトなどを出すならここ
				// deathParticles_->Emit(enemy->GetTranslation());
			}
		}
	}
}

void GameScene::CheckPlayerAndDoor() {
	// 自キャラのAABBを取得
	Player* player = stage_->GetPlayer();
	AABB playerAABB = player->GetAABB();
	// ドアの座標
	Door* door = stage_->GetDoor();
	AABB doorAABB = door->GetAABB();

	// 自キャラとドアの当たり判定
	if (IsCollision(playerAABB, doorAABB)) {
		// 次のステージへ進む
		stageNo_++;

		// ステージ3までは次へ、それ以降はクリア画面へ
		if (stageNo_ <= 3) {
			LoadStage(); // 次のJSONを読み込んでリセット
		} else {
			phase_ = Phase::kClear; // 全ステージクリア！
		}
	}
}

void GameScene::CheckPlayerAndBlocks() {
	// 判定対象1と2の座標
	AABB playerAABB, blockAABB;

	// プレイヤーのAABBと現在の速度を取得
	Player* player = stage_->GetPlayer();
	playerAABB = player->GetAABB();
	Vector3 playerVelocity = player->GetVelocity();

	// 全ての衝突判定ブロックと自キャラの当たり判定
	for (WorldTransform* block : stage_->GetBlocks()) {
		// ブロックのAABBを取得
		blockAABB = GetAABB(*block);

		// AABB同士の交差判定
		if (IsCollision(playerAABB, blockAABB)) {

			// 衝突を解決し、プレイヤーの移動量を修正

			// 1. 衝突を解消するために必要な最小移動ベクトル(MTV)を取得
			Vector3 resolveVector = CalculateAABBOverlap(playerAABB, blockAABB);

			// 2. プレイヤーの座標をMTVの分だけ移動させて、めり込みを解除する
			// resolveVectorには、XかZのどちらか一方にのみ、押し戻し量が入っている（MTV）
			Vector3 pushBackVector = player->GetTranslation();
			pushBackVector += resolveVector;
			player->SetTranslation(pushBackVector);

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
	player->SetVelocity(playerVelocity);
}

void GameScene::LoadStage() {
	// 古いステージデータがあれば一旦きれいにする
	if (stage_) {
		delete stage_;
	}
	stage_ = new Stage();
	// モデルを渡して初期化
	stage_->Initialize(model_, modelAttack_, modelDoor_, modelBlock_, modelEnemy_, modelEnemyBullet_);

	// 文字列を組み立てる (例: "Resources/stage1.json")
	std::string stageFileName = "stage" + std::to_string(stageNo_) + ".json";

	// 読み込み実行
	stage_->Load(stageFileName);

	// カメラのターゲットなどをリセット（必要に応じて）
	cameraController_->SetTarget(stage_->GetPlayer());
	cameraController_->Reset();
}

void GameScene::ChangePhase() {
	Player* player = stage_->GetPlayer();
	switch (phase_) {
	case Phase::kPlay:
		if (player->IsDead()) {
			// 死亡演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			const Vector3& deathParticlesPosition = player->GetWorldPosition();

			// 生成処理
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(modelParticle_, &camera_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:

		break;
	}
}
