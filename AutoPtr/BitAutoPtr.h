#ifndef BIT_AUTO_PTR_H_
#define BIT_AUTO_PTR_H_

#define BIT_AUTO_PTR_MALLOC_ERR               1

class BitAutoPtrLock
{
public:
   BitAutoPtrLock() {}
   ~BitAutoPtrLock() {}
   void Lock() {}
   void UnLock() {}
};

template<class T>
class BitAutoPtr
{
private:
   class Data
   {
   public:
      int  m_count;
      T    *m_pT;
   private:
      BitAutoPtrLock m_lock;
   public:
      Data() : m_count(0), m_pT(NULL) {}
      void Lock() { m_lock.Lock(); }
      void UnLock() { m_lock.UnLock(); }
   };

private:
   Data *m_pData;
   BitAutoPtrLock m_lock;
   bool m_isOnlyShare;

   void Lock() { m_lock.Lock(); }
   void UnLock() { m_lock.UnLock();  }

private:

   Data *GetNewData() const {
      Data *pData = new Data();
      if (pData == NULL) {
         throw BIT_AUTO_PTR_MALLOC_ERR;
      }
      pData->m_count = 1;
      pData->m_pT = NULL;
      return pData;
   }

   void Assign(const BitAutoPtr<T> &bitPtr) {
      BitAutoPtr<T> *pBitPtr = (BitAutoPtr<T> *)&bitPtr;
      m_pData = pBitPtr->AddRef();
      m_isOnlyShare = pBitPtr->IsShare();
   }

   void Init() { m_pData = NULL; m_isOnlyShare = false;  }

   void ReleaseData() {
      if (m_pData == NULL) {
         return;
      }
      m_pData->Lock();

      m_pData->m_count -= 1;
      if (m_pData->m_count <= 0) {
         if (m_pData->m_pT != NULL) {
            delete m_pData->m_pT;
            m_pData->m_pT = NULL;
         }
         delete m_pData;
      }
      m_pData->UnLock();
      m_pData = NULL;
   }

   BitAutoPtr<T> Clone() const {
      BitAutoPtr<T> ret;
      if (m_pData != NULL) {
         if (m_pData->m_pT != NULL) {
            T *pT = new T();
            if (pT == NULL) {
               throw BIT_AUTO_PTR_MALLOC_ERR;
            }
            ret = BitAutoPtr<T>(pT);
            *pT = *m_pData->m_pT;
         }
      }
      return ret;
   }

public:
   bool IsShare() { return m_isOnlyShare; }

   Data *AddRef() {
      if (m_pData == NULL) {
         return NULL;
      }
      m_pData->Lock();
      m_pData->m_count++;
      m_pData->UnLock();
      return m_pData;
   }

   unsigned int GetCount() {
      int ret = 0;
      Lock();
      if (m_pData == NULL) {
         ret = 0;
      } else {
         ret = m_pData->m_count;
      }
      UnLock();
      return ret;
   }

   BitAutoPtr() { Init(); }

   BitAutoPtr(const BitAutoPtr<T> &bitPtr) {
      Init();
      *this = bitPtr;
   }

   BitAutoPtr(T *pT, bool isShare = false) {
      Init();
      m_pData = GetNewData();
      m_pData->m_pT = pT;
      m_isOnlyShare = isShare;
   }

   ~BitAutoPtr() {
      Lock();
      ReleaseData();
      UnLock();
   }

   BitAutoPtr<T> &operator= (const BitAutoPtr<T> &bitPtr) {
      Lock();
      if (m_pData != NULL) {
         ReleaseData();
      }
      Assign(bitPtr);
      UnLock();
      return *this;
   }

   T *operator->() {
      BitAutoPtr<T> clonePtr;
      bool isClone = false;

      m_pData->Lock();
      if (m_pData->m_count > 1) {
         m_pData->m_count -= 1;
         isClone = true;
         clonePtr = Clone();
      }
      m_pData->UnLock();

      if (isClone) {
         Lock();
         Assign(clonePtr);
         UnLock();
      }
      return m_pData->m_pT;
   }

   const T *operator->() const {
      return m_pData->m_pT;
   }
};

#endif // BIT_AUTO_PTR_H_
