//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_k_replacer.cpp
//
// Identification: src/buffer/lru_k_replacer.cpp
//
// Copyright (c) 2015-2022, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_k_replacer.h"

#include <stdexcept>

namespace bustub {

LRUKReplacer::LRUKReplacer(size_t num_frames, size_t k) : replacer_size_(num_frames), k_(k) {
  if (k_ == 0) {
    throw std::invalid_argument("LRU-K requires k to be greater than zero");
  }
}

/**
 * 尝试逐出一个页面。
 * @param frame_id 指向将要被逐出的帧ID的指针。
 * @return 如果成功逐出页面，返回true；否则返回false。
 */
auto LRUKReplacer::Evict(frame_id_t *frame_id) -> bool {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查当前是否有可逐出的页面
  if (curr_size_ == 0) {
    return false;  // 没有可逐出的页面
  }

  // 优先从历史列表中逐出页面
  // 如果历史列表中没有可逐出的页面，尝试从缓存列表中逐出
  auto victim = frame_info_.end();
  for (auto it = frame_info_.begin(); it != frame_info_.end(); ++it) {
    // 检查页面是否可逐出
    if (!it->second.evictable_) {
      continue;
    }

    if (victim == frame_info_.end()) {
      victim = it;
      continue;
    }

    const auto &candidate_history = it->second.history_;
    const auto &victim_history = victim->second.history_;
    const bool candidate_is_infinite = candidate_history.size() < k_;
    const bool victim_is_infinite = victim_history.size() < k_;

    if (candidate_is_infinite != victim_is_infinite) {
      if (candidate_is_infinite) {
        victim = it;
      }
      continue;
    }

    if (candidate_history.front() < victim_history.front()) {
      victim = it;
    }
  }

  if (victim == frame_info_.end()) {
    return false;  // 没有页面被逐出
  }

  // 重置访问次数并从列表中移除
  *frame_id = victim->first;
  frame_info_.erase(victim);
  curr_size_--;
  return true;
}

/**
 * 记录对指定页面的访问。
 * @param frame_id 被访问的帧ID。
 */
void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查frame_id的有效性
  CheckFrameId(frame_id);

  // 增加访问次数
  auto &info = frame_info_[frame_id];
  info.history_.push_back(current_timestamp_);
  current_timestamp_++;
  if (info.history_.size() > k_) {
    info.history_.pop_front();
  }

  // 根据访问次数将页面移动到适当的列表
  // 未达到k次访问，加入或保留在历史列表
  // 如果访问次数未达到k，且frame不在历史列表中，将其添加到历史列表
  // 达到k次访问，移动到缓存列表
  // 从历史列表中移除
  // 添加到缓存列表
  // 超过k次访问，更新在缓存列表中的位置
  // 将frame移动到缓存列表的前端
}

/**
 * 设置指定页面的逐出状态。
 * @param frame_id 要设置的帧ID。
 * @param set_evictable 是否可逐出。
 */
void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查frame_id的有效性
  CheckFrameId(frame_id);

  // 如果访问次数为0，忽略操作
  auto it = frame_info_.find(frame_id);
  if (it == frame_info_.end()) {
    return;
  }

  // 更新逐出状态和当前大小
  if (it->second.evictable_ == set_evictable) {
    return;
  }

  it->second.evictable_ = set_evictable;
  curr_size_ += set_evictable ? 1 : -1;
}

/**
 * @brief 从LRUK替换器中移除指定帧ID的函数。
 *
 * @param frame_id 要移除的帧ID。
 *
 * @throws std::exception 如果frame_id大于替换器的大小或其他错误情况。
 */
void LRUKReplacer::Remove(frame_id_t frame_id) {
  std::scoped_lock<std::mutex> lock(latch_);

  // 检查frame_id是否超出替换器的范围
  CheckFrameId(frame_id);

  auto it = frame_info_.find(frame_id);
  if (it == frame_info_.end()) {
    // 如果访问计数为0，则不执行任何操作，直接返回
    return;
  }

  // 如果帧不可驱逐，则抛出异常
  if (!it->second.evictable_) {
    throw std::exception();
  }

  // 根据访问计数决定移除帧的位置
  // 从历史列表中移除
  // 从缓存列表中移除
  frame_info_.erase(it);
  curr_size_--;
  // 更新当前大小、访问计数和帧的可驱逐状态
}

auto LRUKReplacer::Size() -> size_t {
  std::scoped_lock<std::mutex> lock(latch_);
  return curr_size_;
}

void LRUKReplacer::CheckFrameId(frame_id_t frame_id) const {
  if (frame_id < 0 || static_cast<size_t>(frame_id) >= replacer_size_) {
    throw std::out_of_range("frame id is out of range");
  }
}

}  // namespace bustub
