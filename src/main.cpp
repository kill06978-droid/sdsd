#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// 이번 플레이에서 입력 보조가 쓰였는지 (기록 저장 차단용)
static bool g_assistUsed = false;

// 디버그 카운터
static int g_cntHandle = 0;   // GJBaseGameLayer::handleButton 클릭 횟수
static int g_cntPush = 0;     // PlayerObject::pushButton 횟수
static int g_cntDelayed = 0;  // 늦춰진 릴리즈 횟수

static bool assistEnabled() {
    return Mod::get()->getSettingValue<bool>("enabled");
}

static int holdSteps() {
    auto v = Mod::get()->getSettingValue<int64_t>("hold-steps-v2");
    return v < 1 ? 1 : static_cast<int>(v);
}

class $modify(AssistPlayer, PlayerObject) {
    struct Fields {
        int sinceDown = 100000;
        bool pending = false;
    };

    bool pushButton(PlayerButton button) {
        if (PlayLayer::get() && assistEnabled() && button == PlayerButton::Jump) {
            m_fields->sinceDown = 0;
            m_fields->pending = false;
            g_assistUsed = true;
            g_cntPush++;
        }
        return PlayerObject::pushButton(button);
    }

    bool releaseButton(PlayerButton button) {
        // 너무 빨리 뗀 경우 릴리즈를 보류
        if (PlayLayer::get() && assistEnabled() && button == PlayerButton::Jump
            && m_fields->sinceDown < holdSteps()) {
            m_fields->pending = true;
            g_cntDelayed++;
            return false;
        }
        return PlayerObject::releaseButton(button);
    }

    // 스텝마다 호출: 시간 세고, 시간이 지나면 보류한 릴리즈를 실행
    void tickAssist() {
        if (m_fields->sinceDown < 100000) m_fields->sinceDown++;
        if (m_fields->pending && m_fields->sinceDown >= holdSteps()) {
            m_fields->pending = false;
            PlayerObject::releaseButton(PlayerButton::Jump);
        }
    }
};

class $modify(AssistBaseLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (down && PlayLayer::get() && assistEnabled()) g_cntHandle++;
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);

        if (isHalfTick) return;
        auto pl = PlayLayer::get();
        if (!pl || !assistEnabled()) return;

        if (m_player1) static_cast<AssistPlayer*>(m_player1)->tickAssist();
        if (m_player2) static_cast<AssistPlayer*>(m_player2)->tickAssist();

        // 디버그 표시 갱신
        if (auto dbg = typeinfo_cast<CCLabelBMFont*>(pl->getChildByID("input-assist-debug"_spr))) {
                       dbg->setString(fmt::format("H:{} P:{} D:{} S:{}", g_cntHandle, g_cntPush, g_cntDelayed, holdSteps()).c_str());
        }
    }
};

class $modify(AssistPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        g_assistUsed = false;
        g_cntHandle = 0;
        g_cntPush = 0;
        g_cntDelayed = 0;

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        if (assistEnabled()) {
            auto win = CCDirector::get()->getWinSize();

            auto label = CCLabelBMFont::create("INPUT ASSIST ON - NO RECORDS", "bigFont.fnt");
            label->setScale(0.4f);
            label->setColor({255, 80, 80});
            label->setOpacity(210);
            label->setPosition({win.width / 2.f, win.height - 12.f});
            label->setID("input-assist-indicator"_spr);
            this->addChild(label, 1000);

            auto dbg = CCLabelBMFont::create("H:0 P:0 D:0", "bigFont.fnt");
            dbg->setScale(0.35f);
            dbg->setOpacity(180);
            dbg->setPosition({win.width / 2.f, win.height - 26.f});
            dbg->setID("input-assist-debug"_spr);
            this->addChild(dbg, 1000);
        }
        return true;
    }

    // 보조를 사용한 플레이는 완료 처리(기록 저장)를 하지 않음
    void levelComplete() {
        if (g_assistUsed) {
            FLAlertLayer::create(
                "Input Assist",
                "입력 보조가 사용된 플레이라서 완료 기록이 저장되지 않습니다.",
                "OK"
            )->show();
            this->resetLevel();
            return;
        }
        PlayLayer::levelComplete();
    }
};
