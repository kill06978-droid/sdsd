#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

// 이번 플레이에서 입력 보조가 실제로 쓰였는지 (기록 저장 차단용)
static bool g_assistUsed = false;

static bool assistEnabled() {
    return Mod::get()->getSettingValue<bool>("enabled");
}

static int holdSteps() {
    auto v = Mod::get()->getSettingValue<int64_t>("hold-steps");
    return v < 1 ? 1 : static_cast<int>(v);
}

// 클릭을 놓는 동작을 최소 N스텝 뒤로 미룸 -> 판정 허용 범위가 넓어짐
class $modify(AssistBaseLayer, GJBaseGameLayer) {
    struct Fields {
        int sinceDown[2] = {100000, 100000};
        bool pending[2] = {false, false};
        int pendingButton[2] = {0, 0};
    };

    void handleButton(bool down, int button, bool isPlayer1) {
        if (!PlayLayer::get() || !assistEnabled()) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        int i = isPlayer1 ? 0 : 1;

        if (down) {
            m_fields->sinceDown[i] = 0;
            m_fields->pending[i] = false;
            g_assistUsed = true;
            GJBaseGameLayer::handleButton(true, button, isPlayer1);
            return;
        }

        // 너무 빨리 뗀 경우 릴리즈를 보류
        if (m_fields->sinceDown[i] < holdSteps()) {
            m_fields->pending[i] = true;
            m_fields->pendingButton[i] = button;
            return;
        }

        GJBaseGameLayer::handleButton(false, button, isPlayer1);
    }

    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
        if (isHalfTick) return;

        if (!PlayLayer::get() || !assistEnabled()) return;

        for (int i = 0; i < 2; i++) {
            if (m_fields->sinceDown[i] < 100000) m_fields->sinceDown[i]++;

            if (m_fields->pending[i] && m_fields->sinceDown[i] >= holdSteps()) {
                m_fields->pending[i] = false;
                GJBaseGameLayer::handleButton(false, m_fields->pendingButton[i], i == 0);
            }
        }
    }
};

class $modify(AssistPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        g_assistUsed = false;

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // 보조가 켜져 있으면 항상 화면에 표시
        if (assistEnabled()) {
            auto win = CCDirector::get()->getWinSize();
            auto label = CCLabelBMFont::create("INPUT ASSIST ON - NO RECORDS", "bigFont.fnt");
            label->setScale(0.4f);
            label->setColor({255, 80, 80});
            label->setOpacity(210);
            label->setPosition({win.width / 2.f, win.height - 12.f});
            label->setID("input-assist-indicator"_spr);
            this->addChild(label, 1000);
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
