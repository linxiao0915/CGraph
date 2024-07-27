/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: GStaticEngine.cpp
@Time: 2024/7/27 10:40
@Desc: 
***************************/

#include "GStaticEngine.h"

CGRAPH_NAMESPACE_BEGIN

CStatus GStaticEngine::setup(const GSortedGElementPtrSet& elements) {
    CGRAPH_FUNCTION_BEGIN
    element_mat_.clear();

    GElementPtrArr curArr;
    CSize totalSize = 0;
    for (auto element : elements) {
        CGRAPH_ASSERT_NOT_NULL(element)
        element->beforeRun();
        if (element->dependence_.empty()) {
            curArr.push_back(element);
            totalSize++;
        }
    }

    while (!curArr.empty()) {
        element_mat_.emplace_back(curArr);
        GElementPtrArr runnableArr = curArr;
        curArr.clear();
        for (auto element : runnableArr) {
            for (auto cur : element->run_before_) {
                if (0 == (--cur->left_depend_)) {
                    curArr.push_back(cur);
                    totalSize++;
                }
            }
        }
    }

    CGRAPH_RETURN_ERROR_STATUS_BY_CONDITION(totalSize != elements.size(),
                                            "static engine parse error");
    CGRAPH_FUNCTION_END
}


CStatus GStaticEngine::run() {
    CGRAPH_FUNCTION_BEGIN

    for (auto& arr : element_mat_) {
        std::vector<std::future<CStatus>> futures;
        for (auto* element : arr) {
            auto fut = thread_pool_->commit([element] {
                return element->fatProcessor(CFunctionType::RUN);
            }, element->getBindingIndex());
            futures.emplace_back(std::move(fut));
        }

        for (auto& future : futures) {
            status += future.get();
        }
        CGRAPH_FUNCTION_CHECK_STATUS
    }

    CGRAPH_FUNCTION_END
}

CGRAPH_NAMESPACE_END
