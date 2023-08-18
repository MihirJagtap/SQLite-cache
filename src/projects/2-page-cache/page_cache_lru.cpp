#include <iostream>
#include "page_cache_lru.hpp"
#include "utilities/exception.hpp"
#include <limits>

LRUReplacementPageCache::ReplacementPage::ReplacementPage(
    int argPageSize, int argExtraSize, unsigned argPageId, bool argPinned)
    : Page(argPageSize, argExtraSize), pageId(argPageId), pinned(argPinned) {}


LRUReplacementPageCache::LRUReplacementPageCache(int pageSize, int extraSize)
    : PageCache(pageSize, extraSize) {
}

void LRUReplacementPageCache::setMaxNumPages(int maxNumPages) {
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

int LRUReplacementPageCache::getNumPages() const {
  return (int)pages_.size();
}



Page *LRUReplacementPageCache::fetchPage(unsigned pageId, bool allocate) {
  numFetches_++;
  auto pagesIterator = pages_.find(pageId);
  // If the page is already in the cache, return a pointer to the page
  if (pagesIterator != pages_.end()) {
    ++numHits_;
    pagesIterator->second->pinned = true;
    // remove the page pointer from the list
    lruList.remove(pagesIterator->second);
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
//    lruList.push_back(page);
    return page;
  }
      // Check if the cache size exceeds the maximum allowed
  if (getNumPages() >= maxNumPages_) {

      auto pagesIterator = pages_.begin();
      int leastVal = std::numeric_limits<int>::max();
      auto finId = 0;
      bool foundUnpinnedPage = false;
      for (pagesIterator = pages_.begin(); pagesIterator != pages_.end(); ++pagesIterator) {
          if (pagesIterator->second->pinned) {
              continue;
          }
          if (pagesIterator->second->accessVal < leastVal) {
              foundUnpinnedPage = true;
              leastVal = pagesIterator->second->accessVal;
              finId = pagesIterator->second->pageId;
          }
      }
      if (!foundUnpinnedPage) {
          return nullptr;
      }
      //auto page = new ReplacementPage(pageSize_, extraSize_, pageId, true);
      changePageId(pages_.find(finId)->second,pageId);
      return pages_.find(pageId)->second;

       //Remove the least recently used page from cache and LRU list
//      std::list<ReplacementPage*>::iterator it;
//      for (it = lruList.begin(); it != lruList.end(); ++it) {
//        auto currPageId = (*it)->pageId;
//        auto pagesIterator = pages_.find(currPageId);
//        if(pagesIterator != pages_.end()) {
//          if (!pagesIterator->second->pinned) {
//            ReplacementPage *page = pagesIterator->second;
//              changePageId(page,pageId);
////            pages_.erase(pagesIterator);
////            pages_.emplace(pageId, page);
//            lruList.remove(*it);
//            return page;
//          }
//        }
//      }
  }
  return nullptr;
}

void LRUReplacementPageCache::unpinPage(Page *page, bool discard) {
  auto *newpage = (ReplacementPage *)page;
  // If discard is true or the number of pages in the cache is greater than the
  // maximum, discard the page. Otherwise, unpin the page.
  if (discard || getNumPages() > maxNumPages_) {
    pages_.erase(newpage->pageId);
    //lruList.remove(newpage);
    delete page;
  } else {
    newpage->pinned = false;
    // changed after this
    newpage->accessVal = counter;
    counter++;
    // changed before this
    lruList.push_back(newpage);

  }


}

void LRUReplacementPageCache::changePageId(Page *page, unsigned newPageId) {
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

void LRUReplacementPageCache::discardPages(unsigned pageIdLimit) {
  for (auto pagesIterator = pages_.begin(); pagesIterator != pages_.end();) {
    if (pagesIterator->second->pageId >= pageIdLimit) {
      delete pagesIterator->second;
      pagesIterator = pages_.erase(pagesIterator);
    } else {
      ++pagesIterator;
    }
  }
}
