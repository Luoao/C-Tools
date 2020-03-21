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
public:
   typedef void (*BIT_AUTO_DEL_PTR)(T *pT);

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
   BIT_AUTO_DEL_PTR m_delFun;
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
      m_delFun = pBitPtr->GetDelFun();
   }

   void Init() { m_pData = NULL; m_isOnlyShare = false; m_delFun = NULL;  }

   void ReleaseData() {
      if (m_pData == NULL) {
         return;
      }
      m_pData->Lock();

      m_pData->m_count -= 1;
      if (m_pData->m_count <= 0) {
         if (m_pData->m_pT != NULL) {
            if (m_delFun != NULL) {
               m_delFun(m_pData->m_pT);
            }
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
            T *pT = new T(*m_pData->m_pT);
            if (pT == NULL) {
               throw BIT_AUTO_PTR_MALLOC_ERR;
            }
            ret = BitAutoPtr<T>(pT);
         }
      }
      return ret;
   }

public:
   bool IsShare() { return m_isOnlyShare; }
   BIT_AUTO_DEL_PTR GetDelFun() { return m_delFun; };

   Data *AddRef() {
      if (m_pData == NULL) {
         return NULL;
      }
      m_pData->Lock();
      m_pData->m_count++;
      m_pData->UnLock();
      return m_pData;
   }

   T *GetData() const {
      if (m_pData == NULL) { return NULL; }
      return m_pData->m_pT;
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

   BitAutoPtr(T *pT, BIT_AUTO_DEL_PTR pDelFun = NULL, bool isShare = false) {
      Init();
      m_pData = GetNewData();
      m_pData->m_pT = pT;
      m_isOnlyShare = isShare;
      m_delFun = pDelFun;
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
