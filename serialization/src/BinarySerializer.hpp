#pragma once

#include "Serializer.hpp"

namespace sprawl
{
	namespace serialization
	{
		class BinarySerializerBase : virtual public SerializerBase
		{
		public:
			static const int headerSize = sizeof(int32_t)*3;

			typedef class BinarySerializer serializer_type;
			typedef class BinaryDeserializer deserializer_type;
			uint32_t ComputeChecksum()
			{
				if(m_bWithMetadata)
					return compute_checksum( m_data + headerSize, m_size-headerSize);
				else
					return compute_checksum( m_data, m_size );
			}
			uint32_t GetChecksum() { return m_checksum; }
			virtual void SetVersion(uint32_t i) override
			{
				m_version = i;
				if(IsSaving() && m_bWithMetadata)
				{
					//Update version metadata
					memcpy(m_data + sizeof(int32_t), &i, sizeof(i));
				}
			}
			virtual void Reset() override
			{
				if(IsSaving())
				{
					m_capacity = 256;
					m_data = new char[m_capacity];
					if(m_bWithMetadata)
					{
						m_pos = headerSize;
						m_size = m_pos;
						//Zero out the entire contents
						memset(m_data, 0, m_capacity);

						m_checksum = 0;
						m_checksum_stale = false;

						//Size, first int
						memcpy( m_data, &m_size, sizeof(int32_t) );
						//Version, second int
						memcpy(m_data + sizeof(int32_t), &m_version, sizeof(m_version));
						//Checksum, third int
						memcpy( m_data + sizeof(int32_t)*2, &m_checksum, sizeof(int32_t) );
					}
					else
					{
						m_pos = 0;
						m_size = 0;
						memset(m_data, 0, m_capacity);
					}
				}
				else
				{
					if(m_bWithMetadata)
						m_pos = headerSize;
					else
						m_pos = 0;
				}
			}
			virtual const char *Data() override
			{
				if(m_checksum_stale && m_bWithMetadata)
				{
					m_checksum = compute_checksum( m_data + headerSize, m_size-headerSize);
					memcpy( m_data + sizeof(int32_t)*2, &m_checksum, sizeof(int32_t) );
					m_checksum_stale = false;
				}
				return m_data;
			}
			virtual std::string Str() override
			{
				if(m_checksum_stale && m_bWithMetadata)
				{
					m_checksum = compute_checksum( m_data + headerSize, m_size-headerSize);
					memcpy( m_data + sizeof(int32_t)*2, &m_checksum, sizeof(int32_t) );
					m_checksum_stale = false;
				}
				return std::string(m_data, m_size); }
			bool More(){ return m_pos < m_size; }
		protected:
			char *m_data;
			uint32_t m_pos;
			uint32_t m_capacity;
			uint32_t m_checksum;
			bool m_checksum_stale;

			uint32_t compute_checksum(const char *data, int len)
			{
				uint32_t hash = 0;

				for(int i=0; i<len; i++)
				{
					hash = ((hash << 5) + hash) ^ data[i];
				}

				return hash;
			}

			using SerializerBase::serialize;
			void serialize_impl(void *var, const size_t bytes, bool /*PersistToDB*/)
			{
				if(!m_bIsValid)
				{
					throw ex_invalid_data();
				}
				if(IsSaving())
				{
					if(m_pos + bytes >= m_capacity*0.75)
					{
						int oldcap = m_capacity;
						m_capacity = m_capacity * 2 + 1;
						char *newdata = new char[m_capacity];
						memcpy(newdata, m_data, oldcap);
						delete[] m_data;
						m_data = newdata;
					}
					memcpy( m_data + m_pos, var, bytes );

					m_size += bytes;
					if(m_bWithMetadata)
					{
						memcpy( m_data, &m_size, sizeof(int32_t) );
						m_checksum_stale = true;
					}
				}
				else
				{
					if(m_bInitialized && m_pos + bytes > m_size)
					{
						throw ex_serializer_overflow();
					}
					memcpy( var, m_data + m_pos, bytes );
				}

				m_pos += bytes;
			}
			virtual void serialize(int *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(long int *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(long long int *var, const size_t bytes, const std::string&, bool PersistToDB)  override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(short int *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(char *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(float *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(double *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(long double *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(bool *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(unsigned int *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(unsigned long int *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(unsigned long long int *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(unsigned short int *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(unsigned char *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				serialize_impl(reinterpret_cast<void*>(var), bytes, PersistToDB);
			}

			virtual void serialize(std::string *var, const size_t bytes, const std::string&, bool PersistToDB) override
			{
				if(IsLoading())
				{
					char data[bytes];
					serialize_impl(data, bytes, PersistToDB);
					*var = std::string(data, bytes);
				}
				else
				{
					serialize_impl(const_cast<char*>(var->c_str()), bytes, PersistToDB);
				}
			}

			BinarySerializerBase() : SerializerBase(), m_data(NULL), m_pos(0), m_capacity(0), m_checksum(0), m_checksum_stale(false) {}
			virtual ~BinarySerializerBase()
			{
				if(m_data != NULL)
				{
					delete[] m_data;
					m_data = NULL;
				}
			}
		private:
			BinarySerializerBase(const SerializerBase&);
			BinarySerializerBase &operator=(const SerializerBase&);
		};

		class BinarySerializer : public BinarySerializerBase, public Serializer
		{
		public:
			using Serializer::operator%;

			virtual SerializerBase &operator%(SerializationData<Serializer> &&var) override
			{
				std::string str = var.val.Str();
				*this % prepare_data(str, var.name, var.PersistToDB);
				return *this;
			}

			virtual SerializerBase &operator%(SerializationData<BinarySerializer> &&var) override
			{
				std::string str = var.val.Str();
				*this % prepare_data(str, var.name, var.PersistToDB);
				return *this;
			}

			//Reserve the first sizeof(int32_t)*3 bytes of space to hold metadata (size, version, and checksum).
			BinarySerializer()
			{
				m_bIsLoading = false;
				m_bIsBinary = true;
				m_capacity = 256;
				m_data = new char[m_capacity];
				if(m_bWithMetadata)
				{
					m_pos = headerSize;
					m_size = m_pos;
					//Make sure metadata info is empty.
					memset(m_data, 0, headerSize);
					m_checksum = 0;

					//Size, first int
					memcpy( m_data, &m_size, sizeof(int32_t) );
					//Version, second int
					memcpy(m_data + sizeof(int32_t), &m_version, sizeof(m_version));
					//Checksum, third int
					memcpy( m_data + sizeof(int32_t)*2, &m_checksum, sizeof(int32_t) );
				}
				m_bInitialized = true;
			}

			BinarySerializer(bool)
			{
				m_bIsLoading = false;
				m_bIsBinary = true;
				m_capacity = 256;
				m_bWithMetadata = false;
				m_data = new char[m_capacity];
				m_bInitialized = true;
			}

			virtual ~BinarySerializer() {};
		protected:
			virtual SerializerBase* GetAnother(const std::string& /*data*/) override { throw std::exception(); }
			virtual SerializerBase* GetAnother() override { return new BinarySerializer(false); }
		};

		class BinaryDeserializer : public BinarySerializerBase, public Deserializer
		{
		public:
			using Deserializer::operator%;
			virtual SerializerBase &operator%(SerializationData<Deserializer> &&var) override
			{
				std::string str;
				*this % str;
				var.val.Data(str);
				return *this;
			}
			virtual SerializerBase &operator%(SerializationData<BinaryDeserializer> &&var) override
			{
				std::string str;
				*this % str;
				var.val.Data(str);
				return *this;
			}

			using BinarySerializerBase::Data;
			virtual void Data(const std::string &str) override
			{
				if(m_data != NULL)
				{
					delete[] m_data;
				}
				m_bIsValid = true;
				if(m_bWithMetadata)
				{
					m_data = new char[headerSize];
					m_pos = 0;
					memcpy(m_data, str.c_str(), headerSize);
					serialize(m_size, sizeof(int32_t), "", false);
					serialize(m_version, sizeof(int32_t), "", false);
					serialize(m_checksum, sizeof(int32_t), "", false);
					delete[] m_data;
					m_pos = headerSize;
					m_data = new char[m_size];
					memcpy(m_data, str.c_str(), m_size);
					unsigned int computed_checksum = compute_checksum(m_data + headerSize, m_size-headerSize);
					if(computed_checksum != m_checksum)
					{
						m_bIsValid = false;
					}
				}
				else
				{
					m_size = str.size();
					m_pos = 0;
					m_data = new char[m_size];
					memcpy(m_data, str.c_str(), m_size);
				}
				m_bInitialized = true;
			}
			BinaryDeserializer(const std::string &data)
			{
				m_bIsValid = true;
				m_bIsLoading = true;
				m_bIsBinary = true;
				Data(data);
			}

			BinaryDeserializer(const std::string& data, bool)
			{
				m_bIsValid = true;
				m_bIsLoading = true;
				m_bIsBinary = true;
				m_bWithMetadata = false;
				Data(data);
			}

			BinaryDeserializer()
			{
				m_bIsLoading = true;
				m_bIsBinary = true;
				m_bIsValid = false;
			}
			virtual ~BinaryDeserializer(){}

			virtual SerializerBase* GetAnother(const std::string& data) override { return new BinaryDeserializer(data, false); }
			virtual SerializerBase* GetAnother() override { throw std::exception(); }
		};
	}
}
