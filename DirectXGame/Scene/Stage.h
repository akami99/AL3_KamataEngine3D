#pragma once
#include "KamataEngine.h"
#include "Player/Player.h"
#include "Enemy/Enemy.h"
#include "Door.h"
#include <list>
#include <vector>

class Player; // 前方宣言

class Stage {
public:
    // デストラクタ（ここでリストの中身を削除する）
    ~Stage();

    // 初期化（モデルを受け取って配置を行う）
	void Initialize(KamataEngine::Model* modelPlayer, KamataEngine::Model* modelAttack, KamataEngine::Model* modelDoor, KamataEngine::Model* modelBlock, KamataEngine::Model* modelEnemy, KamataEngine::Model* modelEnemyBullet);

    // 更新
    void Update();

    // 描画
    void Draw(const KamataEngine::Camera& camera);

    // レベルデータを読み込む関数
    void Load(const std::string& fileName);

    // プレイヤーの情報を敵にセットする
    void SetPlayer(Player* player) { player_ = player; }

    // --- ゲッター（GameSceneで当たり判定をするために必要） ---

    // ブロックのリストを取得
    const std::list<KamataEngine::WorldTransform*>& GetBlocks() const { return blocks_; }
	// 床のワールド変換を取得
	KamataEngine::WorldTransform* GetFloorTransform() const { return floorTransform_; }

    // プレイヤーを取得
    Player* GetPlayer() const { return player_; }
    // 敵のリストを取得
    const std::list<Enemy*>& GetEnemies() const { return enemies_; }
    // ドアを取得
    Door* GetDoor() const { return door_; }

private:
    // プレイヤーの参照（敵に渡すため）
    Player* player_ = nullptr;

    // ゴール用のドア
    Door* door_ = nullptr;

    // ステージにあるブロックのリスト
    std::list<KamataEngine::WorldTransform*> blocks_;
	// 床のワールド変換（高さ取得用）
    KamataEngine::WorldTransform* floorTransform_ = nullptr;

    // ステージにいる敵のリスト
    std::list<Enemy*> enemies_;
    
    // モデルを保持しておく
    KamataEngine::Model* modelPlayer_ = nullptr;
    KamataEngine::Model* modelAttack_ = nullptr;
    KamataEngine::Model* modelDoor_ = nullptr;
    KamataEngine::Model* modelBlock_ = nullptr; 
    KamataEngine::Model* modelEnemy_ = nullptr;
	KamataEngine::Model* modelEnemyBullet_ = nullptr;
};