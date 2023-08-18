#include "page_cache_lru_2.hpp"
#include "utilities/exception.hpp"
#include <limits>


LRU2ReplacementPageCache::ReplacementPage::ReplacementPage(
    int argPageSize, int argExtraSize, unsigned argPageId, bool argPinned)
    : Page(argPageSize, argExtraSize), pageId(argPageId), pinned(argPinned) {}


LRU2ReplacementPageCache::LRU2ReplacementPageCache(int pageSize, int extraSize)
    : PageCache(pageSize, extraSize) {
}

void LRU2ReplacementPageCache::setMaxNumPages(int maxNumPages) {
  maxNumPages_ = maxNumPages;

  // Discard unpinned pages until the number of pages in the cache is less than
  // or equal to `maxNumPages_` or only pinned pages remain.
  for (auto pagesIterator = pages_.begin(); pagesIterator != pages_.end();) {
    if (getNumPages() <= maxNumPages_) {
      break;
    }

    if (!pagesIterator->second->pinned) {
      delete pagesIterator->second;
      pagesIterator = pages_.erase(pagesIterator);
    } else {
      ++pagesIterator;
    }
  }
}

int LRU2ReplacementPageCache::getNumPages() const {
  return (int)pages_.size();
}



Page *LRU2ReplacementPageCache::fetchPage(unsigned pageId, bool allocate) {
  numFetches_++;
  auto pagesIterator = pages_.find(pageId);
  // If the page is already in the cache, return a pointer to the page
  if (pagesIterator != pages_.end()) {
    ++numHits_;
    pagesIterator->second->pinned = true;
    return pagesIterator->second;
  }
  // If allocate is false, return a null pointer.
  if (!allocate) {
    return nullptr;
  }
  // If allocate is true, examine the number of pages in the cache.
  // If the number of pages in the cache is less than the maximum, allocate and return a pointer to a new page.
  if (getNumPages() < maxNumPages_) {
    auto page = new ReplacementPage(pageSize_, extraSize_, pageId, true);
    pages_.emplace(pageId, page);
    return page;
  }
  pagesIterator = pages_.begin();
      // Check if the cache size exceeds the maximum allowed
  if (getNumPages() >= maxNumPages_) {
      // Remove the least recently used page from cache and LRU list
      int lastMax = std::numeric_limits<int>::max();
      auto finId = 0;
      bool onlyOneAccess = false;
      auto oneAccessId = 0;
      auto oneAccessMax = std::numeric_limits<int>::max();
      bool foundUnpinnedPage = false;
      for (pagesIterator = pages_.begin(); pagesIterator != pages_.end(); ++pagesIterator) {
          if (!pagesIterator->second->pinned) {
            foundUnpinnedPage = true;

            if (pagesIterator->second->secondAccess == 0) {
                if (pagesIterator->second->firstAccess < oneAccessMax && pagesIterator->second->firstAccess != 0) {
                    onlyOneAccess = true;
                    oneAccessMax = pagesIterator->second->firstAccess;
                    oneAccessId = pagesIterator->second->pageId;
                }
            }
            else if (pagesIterator->second->secondAccess < lastMax) {  // secondAccess
                lastMax = pagesIterator->second->secondAccess;
                finId = pagesIterator->second->pageId;
            // this is a potential page.
            }
          }
      }
      if (!foundUnpinnedPage) {
          return nullptr;
      }
      auto page = new ReplacementPage(pageSize_, extraSize_, pageId, true);

      if (onlyOneAccess) {
          pages_.erase(oneAccessId);
      }
      else {
          pages_.erase(finId);
      }
      pages_.emplace(pageId, page);
      return page;
  }
  return nullptr;
}

void LRU2ReplacementPageCache::unpinPage(Page *page, bool discard) {
  auto *newpage = (ReplacementPage *)page;
  // If discard is true or the number of pages in the cache is greater than the
  // maximum, discard the page. Otherwise, unpin the page.
  if (discard || getNumPages() > maxNumPages_) {
    pages_.erase(newpage->pageId);
    // lruList.remove(newpage);
    delete page;
  } else {
    newpage->pinned = false;
    if (newpage->firstAccess == 0) {
        newpage->firstAccess = counter;
        counter++;
    }
    else {
        int accVal = newpage->firstAccess;
        newpage->firstAccess = counter;
        newpage->secondAccess = accVal;
        counter++;
    }
  }
}

void LRU2ReplacementPageCache::changePageId(Page *page, unsigned newPageId) {
  auto *newpage = (ReplacementPage *)page;

  // Remove the old page ID from `pages_` and change the page ID.
  pages_.erase(newpage->pageId);
  newpage->pageId = newPageId;

  // Attempt to insert a page with page ID `newPageId` into `pages_`.
  auto [pagesIterator, success] = pages_.emplace(newPageId, newpage);


  // If a page with page ID `newPageId` is already in the cache, discard it.
  if (!success) {
    delete pagesIterator->second;
    pagesIterator->second = newpage;
  }
}

void LRU2ReplacementPageCache::discardPages(unsigned pageIdLimit) {
  for (auto pagesIterator = pages_.begin(); pagesIterator != pages_.end();) {
    if (pagesIterator->second->pageId >= pageIdLimit) {
      delete pagesIterator->second;
      pagesIterator = pages_.erase(pagesIterator);
    } else {
      ++pagesIterator;
    }
  }
}