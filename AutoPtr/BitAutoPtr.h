#ifndef BIT_AUTO_PTR_H_
#define BIT_AUTO_PTR_H_

template<class T>
class BitAutoPtr
{
private:
   class Data
   {
   public:
      int m_count;
      T *m_pT;
   public:
      Data() : m_count(0), m_pT(NULL) {}
   };

private:
   Data *m_pData;
   Data *GetData() const { return m_pData; }
   Data *GetNewData() const
   {
      Data *pData = new Data();
      if (pData == NULL)
      {
         throw "New faild";
      }
      pData->m_count = 1;
      pData->m_pT = NULL;
      return pData;
   }

public:
   BitAutoPtr() : m_pData(NULL) {}
   BitAutoPtr(const BitAutoPtr<T> &bitPtr)
   {
      m_pData = NULL;
      *this = bitPtr;
   }
   BitAutoPtr(T *pT)
   {
      m_pData = GetNewData();
      m_pData->m_pT = pT;
   }

   ~BitAutoPtr()
   {
      if (m_pData != NULL)
      {
         m_pData->m_count = m_pData->m_count - 1;
         if (m_pData->m_count <= 0)
         {
            if (m_pData->m_pT != NULL)
            {
               delete m_pData->m_pT;
               m_pData->m_pT = NULL;
            }
            delete m_pData;
            m_pData = NULL;
         }
      }
   }

   BitAutoPtr<T> Clone()
   {
      if (m_pData != NULL && m_pData->m_pT != NULL)
      {
         T *pT = new T();
         if (pT == NULL)
         {
            throw "New faild";
         }
         BitAutoPtr<T> bitPtr(pT);
         *pT = *m_pData->m_pT;
         return bitPtr;
      }
      else
      {
         return BitAutoPtr<T>();
      }
   }

   BitAutoPtr<T> &operator=(const BitAutoPtr<T> &bitPtr)
   {
      if (m_pData != NULL)
      {
         m_pData->m_count = m_pData->m_count - 1;
      }
      Data *pData = bitPtr.GetData();
      if (pData != NULL)
      {
         m_pData = pData;
         m_pData->m_count = m_pData->m_count + 1;
      }
      return *this;
   }

   T *operator->()
   {
      if (m_pData->m_count > 1)
      {
         *this = Clone();
      }
      return m_pData->m_pT;
   }

   const T *operator->() const
   {
      return m_pData->m_pT;
   }
};

#endif // BIT_AUTO_PTR_H_