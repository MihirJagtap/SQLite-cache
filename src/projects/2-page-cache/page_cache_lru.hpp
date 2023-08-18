#ifndef CS564_PROJECT_PAGE_CACHE_LRU_HPP
#define CS564_PROJECT_PAGE_CACHE_LRU_HPP

#include "page_cache.hpp"
#include <unordered_map>
#include <list>

class LRUReplacementPageCache : public PageCache {
public:
  LRUReplacementPageCache(int pageSize, int extraSize);

  void setMaxNumPages(int maxNumPages) override;

  [[nodiscard]] int getNumPages() const override;

  Page *fetchPage(unsigned int pageId, bool allocate) override;

  void unpinPage(Page *page, bool discard) override;

  void changePageId(Page *page, unsigned int newPageId) override;

  void discardPages(unsigned int pageIdLimit) override;

private:
    struct ReplacementPage : public Page {
    ReplacementPage(int pageSize, int extraSize, unsigned pageId,
                          bool pinned);

    unsigned pageId;
    bool pinned;
    int accessVal = 0;
    // int lastAccessTime;
  };

  std::unordered_map<unsigned, ReplacementPage *> pages_;
  // unsigned lruTime = 0;
  std::list<ReplacementPage*> lruList;
  int counter = 1;
};

#endif // CS564_PROJECT_PAGE_CACHE_LRU_HPP
