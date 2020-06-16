#include "glistener.h"

void GListener::Accept(std::shared_ptr<GPeer> const &p) {
    // 连上了
    if (p) {
        // 设置超时时长( 经验数值 )避免 fd 泄露
        p->SetTimeoutSeconds(5);
        // 持有
        p->Hold();
    }
}
