#!/bin/bash

outputs_apk_name="HEQ.apk"

# 编译出来的 apk 路径
outputs_apk="$(pwd)/app/build/outputs/apk/mt8163/release/${outputs_apk_name}"

# git 提交的注释
commit_msg="[fix][heq]：
[根本原因]:
[解决方案]:
[问题或者需求ID]:
[升级方式]: 系统更新"

# 各分支 apk 的目录
AC8227="$(pwd)/../APK_RELEASE_AC8227"
AC8257="$(pwd)/../APK_RELEASE_AC8257"
MT8163="$(pwd)/../APK_RELEASE_MT8163"
MT8321="$(pwd)/../APK_RELEASE_MT8321"
UIS8581="$(pwd)/../APK_RELEASE_UIS8581"

# apk 仓库地址
APK_RELEASE_URL="http://192.168.0.223:8081/autoapps/release/HEQ"

if [ ! -f "$outputs_apk" ]; then
  echo -e "\033[31m${outputs_apk} 不存在，退出！！！\033[31m"
  exit
fi

commit_AC8227() {
  if [ ! -d "${AC8227}" ]; then
    echo "${AC8227} 目录不存在！！！自动创建目录并 clone 代码"
    git clone ${APK_RELEASE_URL} -b AC8227 "${AC8227}"
    cd "${AC8227}" && mkdir -p .git/hooks && curl -Lo `git rev-parse --git-dir`/hooks/commit-msg http://192.168.0.223:8081/tools/hooks/commit-msg; chmod +x `git rev-parse --git-dir`/hooks/commit-msg
  fi

  cd ${AC8227}
  git reset --hard
  git pull --rebase origin AC8227
  cp -rf ${outputs_apk} ${AC8227}
  git add "${outputs_apk_name}"
  git commit -m "${commit_msg}"
  git push origin HEAD:refs/for/AC8227
}

commit_AC8257() {
  if [ ! -d "${AC8257}" ]; then
    echo "${AC8257} 目录不存在！！！自动创建目录并 clone 代码"
    git clone ${APK_RELEASE_URL} -b AC8257 "${AC8257}"
    cd "${AC8257}" && mkdir -p .git/hooks && curl -Lo `git rev-parse --git-dir`/hooks/commit-msg http://192.168.0.223:8081/tools/hooks/commit-msg; chmod +x `git rev-parse --git-dir`/hooks/commit-msg
  fi

  cd ${AC8257}
  git reset --hard
  git pull --rebase origin AC8257
  cp -rf ${outputs_apk} ${AC8257}
  git add "${outputs_apk_name}"
  git commit -m "${commit_msg}"
  git push origin HEAD:refs/for/AC8257
}

commit_MT8163() {
  if [ ! -d "${MT8163}" ]; then
    echo "${MT8163} 目录不存在！！！自动创建目录并 clone 代码"
    git clone ${APK_RELEASE_URL} -b MT8163 "${MT8163}"
    cd "${MT8163}" && mkdir -p .git/hooks && curl -Lo `git rev-parse --git-dir`/hooks/commit-msg http://192.168.0.223:8081/tools/hooks/commit-msg; chmod +x `git rev-parse --git-dir`/hooks/commit-msg
  fi

  cd ${MT8163}
  git reset --hard
  git pull --rebase origin MT8163
  cp -rf ${outputs_apk} ${MT8163}
  git add "${outputs_apk_name}"
  git commit -m "${commit_msg}"
  git push origin HEAD:refs/for/MT8163
}

commit_MT8321() {
  if [ ! -d "${MT8321}" ]; then
    echo "${MT8321} 目录不存在！！！自动创建目录并 clone 代码"
    git clone ${APK_RELEASE_URL} -b MT8321 "${MT8321}"
    cd "${MT8321}" && mkdir -p .git/hooks && curl -Lo `git rev-parse --git-dir`/hooks/commit-msg http://192.168.0.223:8081/tools/hooks/commit-msg; chmod +x `git rev-parse --git-dir`/hooks/commit-msg
  fi

  cd ${MT8321}
  git reset --hard
  git pull --rebase origin MT8321
  cp -rf ${outputs_apk} ${MT8321}
  git add "${outputs_apk_name}"
  git commit -m "${commit_msg}"
  git push origin HEAD:refs/for/MT8321
}

commit_UIS8581() {
  if [ ! -d "${UIS8581}" ]; then
    echo "${UIS8581} 目录不存在！！！自动创建目录并 clone 代码"
    git clone ${APK_RELEASE_URL} -b UIS8581 "${UIS8581}"
    cd "${UIS8581}" && mkdir -p .git/hooks && curl -Lo `git rev-parse --git-dir`/hooks/commit-msg http://192.168.0.223:8081/tools/hooks/commit-msg; chmod +x `git rev-parse --git-dir`/hooks/commit-msg
  fi

  cd ${UIS8581}
  git reset --hard
  git pull --rebase origin UIS8581
  cp -rf ${outputs_apk} ${UIS8581}
  git add "${outputs_apk_name}"
  git commit -m "${commit_msg}"
  git push origin HEAD:refs/for/UIS8581
}

# 调用函数，提交对应的分支
commit_AC8227
commit_AC8257
commit_MT8163
commit_MT8321
commit_UIS8581