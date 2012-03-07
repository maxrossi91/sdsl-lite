/* sdsl - succinct data structures library
    Copyright (C) 2009 Simon Gog 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*! \file rank_support_v5.hpp
    \brief rank_support_v5.hpp contains rank_support_v5 that support a sdsl::bit_vector with constant time rank information. 
	\author Simon Gog
*/
#ifndef INCLUDED_SDSL_RANK_SUPPORT_VFIVE
#define INCLUDED_SDSL_RANK_SUPPORT_VFIVE

#include "rank_support.hpp"
#include "rank_support_v.hpp"

//! Namespace for the succinct data structure library.
namespace sdsl{

template<uint8_t, uint8_t>
struct rank_support_v_trait;	

//! A class supporting rank queries in constant time. The implementation is a space saving version of the data structure porposed by Vigna (WEA 2008).
/*! \par Space complexity
 *  \f$ 0.0625n\f$ bits for a bit vector of length n bits.
 * @ingroup rank_support_group 
 */
template<uint8_t b=1, uint8_t pattern_len=1>	
class rank_support_v5 : public rank_support {
	public:
		typedef bit_vector bit_vector_type;	
	private:
	int_vector<64> m_basic_block; // basic block for interleaved storage of superblockrank and blockrank
	public:
		rank_support_v5(const bit_vector *v = NULL);
		rank_support_v5(const rank_support_v5 &rs);
		~rank_support_v5();
		void init(const bit_vector *v=NULL);
		const size_type rank(size_type idx) const;
		const size_type operator()(size_type idx)const;
		const size_type size()const;
		size_type serialize(std::ostream &out)const;
		void load(std::istream &in, const bit_vector *v=NULL);
		void set_vector(const bit_vector *v=NULL);

		//! Assign Operator
		/*! Required for the Assignable Concept of the STL.
		 */
		rank_support_v5& operator=(const rank_support_v5 &rs);
		//! swap Operator
		/*! Swap two rank_support_v5 in constant time.
		 *	All members (excluded the pointer to the supported SDSBitVector) are swapped.
         *
		 *  Required for the Container Concept of the STL.
		 */
		void swap(rank_support_v5 &rs);
		//! Equality Operator
		/*! Two rank_support_v5s are equal if all member variables are equal.
		 *
		 * Required for the Equality Comparable Concept of the STL.
		 * \sa operator!=
		 */
		bool operator==(const rank_support_v5 &rs)const;
		//! Unequality Operator
		/*! Two rank_support_v5s are not equal if any member variable are not equal.
		 *
		 * Required for the Equality Comparable Concept of the STL.
		 * \sa operator==
		 */
		bool operator!=(const rank_support_v5 &rs)const;

#ifdef MEM_INFO
		void mem_info(std::string label="")const{
			if(label=="")
				label="rank";
			size_type bytes = util::get_size_in_bytes(*this);
			std::cout << "list(label = \""<<label<<"\", size = "<< bytes/(1024.0*1024.0) <<")\n";
		}
#endif

};

template<uint8_t b, uint8_t pattern_len>
inline rank_support_v5<b, pattern_len>::rank_support_v5(const bit_vector *v){
	init(v);
}

template<uint8_t b, uint8_t pattern_len>
inline rank_support_v5<b, pattern_len>::rank_support_v5(const rank_support_v5 &rs){
	m_v = rs.m_v;
	m_basic_block = rs.m_basic_block;
}

template<uint8_t b, uint8_t pattern_len>
inline void rank_support_v5<b, pattern_len>::init(const bit_vector *v){
	set_vector(v);
	if( v == NULL or v->empty() )
		return;
	size_type basic_block_size = ((v->capacity() >> 11)+1)<<1; 
	m_basic_block.resize( basic_block_size ); // resize structure for basic_blocks
	if( m_basic_block.empty() )
		return;
	const uint64_t *data = m_v->data();
	size_type i, j=0;
	m_basic_block[0] = m_basic_block[1] = 0;
//	uint64_t carry = 0;
	
	uint64_t carry = rank_support_v_trait<b, pattern_len>::init_carry();
	uint64_t sum = rank_support_v_trait<b, pattern_len>::args_in_the_word(*data, carry), second_level_cnt = 0;
	uint64_t cnt_words=1;
	for(i = 1; i < (m_v->capacity()>>6) ; ++i, ++cnt_words){
		if( cnt_words == 32 ){
			j += 2;
			m_basic_block[j-1] = second_level_cnt;
			m_basic_block[j] 	= m_basic_block[j-2] + sum;
			second_level_cnt = sum = cnt_words = 0;
		}
		else if( (cnt_words%6)==0  )
		{
			// pack the prefix sum for each 6x64bit block into the second_level_cnt
			second_level_cnt |= sum<<(60-12*(cnt_words/6));//  48, 36, 24, 12, 0
		}
		sum += rank_support_v_trait<b, pattern_len>::args_in_the_word(*(++data), carry);
	}

	if( (cnt_words%6)==0 ){
		second_level_cnt |= sum<<(60-12*(cnt_words/6));
	}
	if( cnt_words == 32 ){
		j += 2;
		m_basic_block[j-1] = second_level_cnt;
	    m_basic_block[j]   = m_basic_block[j-2] + sum;
		m_basic_block[j+1] = 0;	
	}else{
		m_basic_block[j+1] = second_level_cnt;
	}
	
}

template<uint8_t b, uint8_t pattern_len>
inline const typename rank_support_v5<b, pattern_len>::size_type rank_support_v5<b, pattern_len>::rank(size_type idx)const{
	const uint64_t *p = m_basic_block.data() + ((idx>>10)&0xFFFFFFFFFFFFFFFEULL);// (idx/2048)*2
	size_type result = *p + ( (*(p+1)>>(60-12*( (idx&0x7FF)/(64*6) )))&0x7FFULL )+ // ( prefix sum of the 6x64bit blocks | (idx%2048)/(64*6)  )
						rank_support_v_trait<b, pattern_len>::word_rank( m_v->data(), idx );
//	std::cerr<<"idx="<<idx<<std::endl;
	idx -= (idx&0x3F);
//	uint32_t to_do = ((idx>>6)&0x1FULL)%6;
	uint8_t to_do = ((idx>>6)&0x1FULL)%6;
	--idx;
	while( to_do ){
		result +=	rank_support_v_trait<b, pattern_len>::full_word_rank( m_v->data(), idx );
		--to_do;
		idx-=64;
	} 
	// could not be accelerated with switch command. 2009-12-04
	return result;
	
}


template<uint8_t b, uint8_t pattern_len>
inline const typename rank_support_v5<b, pattern_len>::size_type rank_support_v5<b, pattern_len>::operator()(size_type idx)const{
	return rank(idx);
}

template<uint8_t b, uint8_t pattern_len>
inline rank_support_v5<b, pattern_len>::~rank_support_v5(){}

template<uint8_t b, uint8_t pattern_len>
inline void rank_support_v5<b, pattern_len>::set_vector(const bit_vector *v){
	m_v = v;
}


template<uint8_t b, uint8_t pattern_len>
inline const typename rank_support_v5<b,pattern_len>::size_type rank_support_v5<b, pattern_len>::size()const{
	return m_v->size();
}

template<uint8_t b, uint8_t pattern_len>
inline typename rank_support_v5<b, pattern_len>::size_type rank_support_v5<b, pattern_len>::serialize(std::ostream &out)const{
	return m_basic_block.serialize(out);
}

template<uint8_t b, uint8_t pattern_len>
inline void rank_support_v5<b, pattern_len>::load(std::istream &in, const bit_vector *v){
	set_vector(v);
	assert(m_v != NULL); // supported bit vector should be known
	m_basic_block.load(in);
}

template<uint8_t b, uint8_t pattern_len>
inline rank_support_v5<b, pattern_len>& rank_support_v5<b, pattern_len>::operator=(const rank_support_v5 &rs){
	if(this != &rs){
		set_vector(rs.m_v);
		m_basic_block = rs.m_basic_block;
	}
	return *this;
}

template<uint8_t b, uint8_t pattern_len>
inline void rank_support_v5<b, pattern_len>::swap(rank_support_v5 &rs){
	if(this != &rs){ // if rs and _this_ are not the same object
		// TODO: swap m_v??? no!!! but the swap of the rank data strucutre has to be made after the swap of the supported bitvectors!!!
//		std::swap(m_v, rs.m_v);
		m_basic_block.swap(rs.m_basic_block);
	}
}

// TODO: == operator remove pointer comparison
template<uint8_t b, uint8_t pattern_len>
inline bool rank_support_v5<b, pattern_len>::operator==(const rank_support_v5 &rs)const{
	if(this == &rs)
		return true;
	return m_basic_block == rs.m_basic_block and *(rs.m_v) == *m_v;
}

template<uint8_t b, uint8_t pattern_len>
inline bool rank_support_v5<b, pattern_len>::operator!=(const rank_support_v5 &rs)const{
	return !(*this == rs);	
}

}// end namespace sds

#endif // end file 