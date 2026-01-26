#pragma once

#define INFRA_STR_IMPL(x) #x
#define INFRA_STR(x) INFRA_STR_IMPL(x)

#define INFRA_CONCAT_IMPL(a, b) a##b
#define INFRA_CONCAT(a, b) INFRA_CONCAT_IMPL(a, b)
