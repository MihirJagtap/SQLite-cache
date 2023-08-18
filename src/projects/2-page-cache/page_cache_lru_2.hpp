#ifndef CS564_PROJECT_PAGE_CACHE_LRU_2_HPP
#define CS564_PROJECT_PAGE_CACHE_LRU_2_HPP

#include "page_cache.hpp"
#include <unordered_map>
#include <list>

class LRU2ReplacementPageCache : public PageCache {
public:
  LRU2ReplacementPageCache(int pageSize, int extraSize);

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
    int firstAccess = 0;
    int secondAccess = 0;
  };

  std::unordered_map<unsigned, ReplacementPage *> pages_;
  //std::list<ReplacementPage*> lruList;
  int counter = 1;


};

#endif // CS564_PROJECT_PAGE_CACHE_LRU_2_HPP
