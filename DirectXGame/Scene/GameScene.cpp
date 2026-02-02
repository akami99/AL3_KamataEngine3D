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
	conditionTextHandle_ = TextureManager::Load("./Resources/startGame/conditionText.png");
	count3Handle_ = TextureManager::Load("./Resources/startGame/count3.png");
	count2Handle_ = TextureManager::Load("./Resources/startGame/count2.png");
	count1Handle_ = TextureManager::Load("./Resources/startGame/count1.png");
	goHandle_ = TextureManager::Load("./Resources/startGame/go.png");
	pauseBGHandle_ = TextureManager::Load("./Resources/pause/pauseBG.png");
	pauseCursorHandle_ = TextureManager::Load("./Resources/pause/cursor.png");
	pauseButtonHandle_ = TextureManager::Load("./Resources/pause/button.png");

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
	spriteConditionText_ = Sprite::Create(conditionTextHandle_, { 640.0f, 360.0f }, { 1.0f, 1.0f, 1.0f, 0.8f }, {0.5f, 0.5f});
	spriteCount3_ = Sprite::Create(count3Handle_, { 640.0f, 360.0f }, { 1.0f, 1.0f, 1.0f, 0.8f }, { 0.5f, 0.5f });
	spriteCount2_ = Sprite::Create(count2Handle_, { 640.0f, 360.0f }, { 1.0f, 1.0f, 1.0f, 0.8f }, { 0.5f, 0.5f });
	spriteCount1_ = Sprite::Create(count1Handle_, { 640.0f, 360.0f }, { 1.0f, 1.0f, 1.0f, 0.8f }, { 0.5f, 0.5f });
	spriteGo_ = Sprite::Create(goHandle_, { 640.0f, 360.0f }, { 1.0f, 1.0f, 1.0f, 0.8f }, { 0.5f, 0.5f });
	spritePauseBG_ = Sprite::Create(pauseBGHandle_, { 640.0f, 360.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.5f, 0.5f });
	spritePauseCursor_ = Sprite::Create(pauseCursorHandle_, { 400.0f, 360.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.5f, 0.5f });
	spritePauseButton_ = Sprite::Create(pauseButtonHandle_, { 20.0f, 20.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });

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
	delete spriteConditionText_;
	spriteConditionText_ = nullptr;
	delete spriteCount3_;
	spriteCount3_ = nullptr;
	delete spriteCount2_;
	spriteCount2_ = nullptr;
	delete spriteCount1_;
	spriteCount1_ = nullptr;
	delete spriteGo_;
	spriteGo_ = nullptr;
	delete spritePauseBG_;
	spritePauseBG_ = nullptr;
	delete spritePauseCursor_;
	spritePauseCursor_ = nullptr;
	delete spritePauseButton_;
	spritePauseButton_ = nullptr;
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
			// フェードが終わったら、次は「開始演出」へ
			phase_ = Phase::kStartProduction;
			startTimer_ = 0.0f;
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
	
	case Phase::kStartProduction:
			// タイマーを進める
			startTimer_ += 1.0f / 60.0f;

			// 全ての演出時間（目的 + カウント + GO）が経過したらゲーム開始
			if (startTimer_ >= kTimeTitleObj + kTimeCountDown + kTimeGo) {
				phase_ = Phase::kPlay;
			}
			// ★重要: ここでは player_->Update() や enemies_.Update() を呼ばない！
			// カメラの更新だけはしておくと、少しカメラが動く演出などを入れられます
			camera_.UpdateMatrix();
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

		// ポーズキー（例: TABキー）が押されたらポーズ状態へ
		if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
			phase_ = Phase::kPause;
			// カーソルを初期位置（Continue）にリセット
			pauseCursor_ = 0;
			// 音楽を一時停止するなどの処理があればここに
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

	case Phase::kPause:
		// --- ポーズ中の処理 ---

		// 1. カーソル移動（上）
		if (Input::GetInstance()->TriggerKey(DIK_W) || Input::GetInstance()->TriggerKey(DIK_UP)) {
			pauseCursor_--;
			if (pauseCursor_ < 0) {
				pauseCursor_ = static_cast<int>(PauseOption::kNumOptions) - 1; // 一番下へ
			}
		}
		// 2. カーソル移動（下）
		if (Input::GetInstance()->TriggerKey(DIK_S) || Input::GetInstance()->TriggerKey(DIK_DOWN)) {
			pauseCursor_++;
			if (pauseCursor_ >= static_cast<int>(PauseOption::kNumOptions)) {
				pauseCursor_ = 0; // 一番上へ
			}
		}

		// 3. 決定操作（SPACE or ENTER）
		if (Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			Input::GetInstance()->Initialize();
			// 選択肢に応じた処理
			PauseOption selection = static_cast<PauseOption>(pauseCursor_);

			if (selection == PauseOption::kContinue) {
				// ゲーム再開
				phase_ = Phase::kPlay;
			} else if (selection == PauseOption::kStageSelect) {
				// ステージセレクトへ戻る処理
				isBackToStageSelect_ = true; // ステージセレクトへ戻るフラグを立てる
				// フェードアウト
				fade_->Start(Fade::Status::FadeOut, kFadeTime);
				phase_ = Phase::kFadeOut;
			} else if (selection == PauseOption::kTitle) {
				// タイトルへ戻る
				isRetire_ = true;
				// フェードアウト
				fade_->Start(Fade::Status::FadeOut, kFadeTime);
				phase_ = Phase::kFadeOut;
			}
		}

		// ポーズキー（TAB）でキャンセル（コンティニューと同じ挙動）
		if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
			phase_ = Phase::kPlay;
		}
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

	// UIや演出の描画
	if (phase_ == Phase::kStartProduction) {

		// 前半: 条件テキストの表示
		if (startTimer_ < kTimeTitleObj) {
			// 例: 「GOALを目指せ」スプライトを描画
			spriteConditionText_->Draw(); 
		}
		// 後半: カウントダウン
		else if (startTimer_ < kTimeTitleObj + kTimeCountDown) {
			// カウントダウン中の経過時間
			float timerInCount = startTimer_ - kTimeTitleObj;
			// 残り時間を計算 (3.0 -> 0.0)
			float restTime = kTimeCountDown - timerInCount;

			// 切り上げで整数にする (3, 2, 1)
			int count = static_cast<int>(std::ceil(restTime));

			// ※念のため0にならないように補正（計算誤差対策）
			if (count < 1) count = 1;

			if (count == 3) {
				// 「3」の画像を描画
				spriteCount3_->Draw();
			} else if (count == 2) {
				// 「2」の画像を描画
				spriteCount2_->Draw();
			} else if (count == 1) {
				// 「1」の画像を描画
				spriteCount1_->Draw();
			}
		}
		// 最後は「GO!」を表示
		else {
			// ここに来るのは、カウントダウンが終わってから kPlay になるまでの1秒間
			// ここで GO を描画します

			spriteGo_->Draw();
		}
	}

	// プレイ中 または ポーズ中ならUIを表示
	if (phase_ == Phase::kPlay || phase_ == Phase::kPause) {
		// 通常のゲームUI（ライフやスコアなど）を描画

	    // 操作説明を表示するキー表示
		spriteTutorialButton_->Draw();
		spritePauseButton_->Draw();
	}

	// ★ポーズ中なら、追加でポーズメニューを描画
	if (phase_ == Phase::kPause) {
		// 1. 半透明の黒背景などを描画して画面を暗くすると見やすい（任意）
		spritePauseBG_->Draw();

		// 2. メニュー文字の描画

		// カーソル位置に合わせて色を変えたり、カーソル画像を描画したりする
		Vector2 cursorPos = { 440.0f, 330.0f + pauseCursor_ * 85.0f };
		spritePauseCursor_->SetPosition(cursorPos);
		spritePauseCursor_->Draw();
		
	}

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
		phase_ = Phase::kClear; 
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

	stage_->Update();

	phase_ = Phase::kStartProduction;
	
	startTimer_ = 0.0f;

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
