/*
Copyright (C) 2011 Adapteva, Inc.
Contributed by Oleg Raikhman <support@adapteva.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program, see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.
*/

#ifndef MEMMANAGER_H
#define MEMMANAGER_H


#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <complex>
using namespace std;



class SAddressRangeConstraint : public scv_constraint_base {
protected :
	scv_smart_ptr<sc_uint<32> > r_max, r_min, min_size, max_size;
public:
	scv_smart_ptr<sc_uint<32> > first,second;


public:
	SCV_CONSTRAINT_CTOR(SAddressRangeConstraint) {


		//t oavoid the SCV warning
		*r_min = 0;
		*r_max = 8;
		*min_size = 16;
		*max_size = MAX_SUB_ROUTINE_SIZE;

		//force randomization from set range
		r_min->disable_randomization();
		r_max->disable_randomization();
		min_size->disable_randomization();
		max_size->disable_randomization();

		// TODO define range
		//SCV_CONSTRAINT( );
		//SCV_CONSTRAINT( );


		SCV_CONSTRAINT( second() >= first() && second() - first() >= min_size()    && second() - first()  <= max_size() && first()  >= r_min() && second()  < r_max()   );

	}
public:

	void SetMinSize(sc_uint<32> sz) {     *min_size = sz; }
	void SetMaxSize(sc_uint<32> sz) {     *max_size = sz; }


	void GenRange(sc_uint<32>  rmin,sc_uint<32>  rmax, pair<sc_uint<32> ,
			sc_uint<32> > &resRange) {

		*r_min = rmin;
		*r_max = rmax;

		this->next();

		resRange.first = first->read();
		resRange.second = second->read();

	}
};

class TMemManager {
private:
	//used for load store operation
	map<unsigned,sc_uint<8> > fAccessMem;

private :
	scv_smart_ptr<sc_uint<32> > genSize;

public:
	TMemManager():addressRangeConstraint("addressRangeConstraint") {
		//    fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(0x40,0xeffff));
		//fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(0x1000,0x5fff));
		//fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(0x100000,0x17ffff));
	//	fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(0x100000,0xffffffff));
		///fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(0x100000,0x110000));

	//	fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(0xfffff000,0xffffffff));
	//	fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(0x100000,0x7fffff));

	}

	//the dma will operate the range list
	friend class TDmaEngine;


public:
	void DumpInitialRangesList(::std::ostream& s= ::std::cout ) {
		vector< pair<sc_uint<32> , sc_uint<32> > >::iterator it;
		s << "LS:";
		for( it = fRangeListForLS.begin() ; it  != fRangeListForLS.end() ; it++ ) {
			s << hex << *it << dec << endl;
			//s << hex << it->first << "," << it->second << dec<< endl;
			//s<< hex << complex<sc_uint<32> >(it->first,it->second)<< dec << endl;
		}
		s << "FETCH:";
		for( it = fRangeListForFetch.begin() ; it  != fRangeListForFetch.end() ; it++ ) {
			s << hex << *it << dec << endl;
			//s << hex << it->first << "," << it->second << dec<< endl;
			//s<< hex << complex<sc_uint<32> >(it->first,it->second)<< dec << endl;
		}
	}
	void ReadInitialRangesList(::std::istream& s= ::std::cin ) {
		vector< pair<sc_uint<32> , sc_uint<32> > >  fRangeList;

		fRangeList.clear();
		pair<  unsigned long, unsigned long > r;
		while(!s.eof() ) {
			s >> hex >> r >> dec;
			if(s.good()) {
				fRangeList.push_back(r);
			}

		}

		sort(fRangeList.begin(),fRangeList.end());

		vector< pair<sc_uint<32> , sc_uint<32> > >::iterator it;
		//assert(fRangeList.size() > 0);

		//read memory selector
		scv_smart_ptr<bool>  ifForFetch;

		for( it = fRangeList.begin() ; it  != fRangeList.end() ; it++ ) {

			cerr << hex << *it << dec << endl;
			if(isFPonly) { //we need the pointer to save the L/S after FP instructions
				if(it==fRangeList.begin()) {
					ifForFetch.write(true);
				} else {
					ifForFetch.write(false);
				}

			} else {
				ifForFetch->next();
			}

			if( (it != fRangeList.begin()) && (it-1)->second < (it)->first  ) {
				if( ifForFetch.read() ) {
					fRangeListForFetch.push_back(*it);

				} else {
					fRangeListForLS.push_back(*it);
				}


			} else {
				if(it == fRangeList.begin()) {
					fRangeListForFetch.push_back(*it);
				} else {
					cerr << hex << "ERROR : range overlaps (it-1)->second < (it)->first" << (it-1)->second << " " << (it)->first  << dec << endl;
					exit(8);
				}
			}
		}


	}

public:

	bool IsNoLSMoreMemory() {
		return fRangeListForLS.empty();
	}
	bool IsNoFetchMoreMemory() {
		return fRangeListForFetch.empty();
	}

	void PrintRange( vector< pair<sc_uint<32> , sc_uint<32> > >  &fRangeList, ::std::ostream& s= ::std::cout ) {
		vector< pair<sc_uint<32> , sc_uint<32> > >::iterator it;

		for( it = fRangeList.begin() ; it  != fRangeList.end() ; it++ ) {
			s << hex << "[" << it->first << "," << it->second << "]" << dec;
		}

		s << endl;

	}

	bool GetRangeIndexForLS(unsigned& ind , pair<sc_uint<32>  ,sc_uint<32> > range ) {

		return (!fRangeListForLS.empty()) && (GetRangeIndex(ind,range,fRangeListForLS));
	}
	bool GetRangeIndexForFetch(unsigned& ind , pair<sc_uint<32>  ,sc_uint<32> > range) {
		return GetRangeIndex(ind,range,fRangeListForFetch);
	}
	//check if range can be valid m return the ind in success ,
	//the ind will be used for remove-update operation
	bool GetRangeIndex(unsigned& ind , pair<sc_uint<32>  ,sc_uint<32> > range,vector< pair<sc_uint<32> , sc_uint<32> > > &fRangeList) {

		//cerr << hex << "range.first second " << range.first << "__" << range.second<< dec  << endl;
		assert(range.first < range.second);
		assert(fRangeList.size());

		bool st = true;
		vector< pair<sc_uint<32> , sc_uint<32> > >::iterator it;
		for( it = fRangeList.begin() ; it  != fRangeList.end() ; it++ ) {
			if( range.first >= it->first && range.second < it->second) {
				ind = ( it - fRangeList.begin());
				break;
			}
		}

		if( it == fRangeList.end() ) { ///not found
			st = false;
		}
		//cerr << " Found " << st <<endl;
		return st;
	}

	void RemoveUpdateRangeLS(unsigned ind, pair<sc_uint<32>,sc_uint<32> > genRange ) {

		cerr <<  "LS RemoveUpdateRange:" << endl;
		RemoveUpdateRange(ind,genRange,fRangeListForLS);
	}
	void RemoveUpdateRangeFetch(unsigned ind, pair<sc_uint<32>,sc_uint<32> > genRange ) {
		cerr <<  "Fetch RemoveUpdateRange:" << endl;
		RemoveUpdateRange(ind,genRange,fRangeListForFetch);
	}

	// 'split' the {rmin .. rmax] to [rmin .. first [ , [ second .. rmax [
	//while resulted interval size should have min MAX_SUB_ROUTINE_SIZE  bytes
	void RemoveUpdateRange(unsigned ind, pair<sc_uint<32>,sc_uint<32> > genRange,vector< pair<sc_uint<32> , sc_uint<32> > > &fRangeList ) {

		assert(ind < fRangeList.size());
		sc_uint<32>  r_min = fRangeList[ind].first;
		sc_uint<32>  r_max = fRangeList[ind].second;

		cerr << " (ind:"<< dec << ind << ")["<< hex << r_min  << ".." << genRange.first;
		cerr << hex << "<-->" << genRange.second << ".." << r_max << "]" << dec << endl;

		///remove list from range
		assert(ind < fRangeList.size());
		fRangeList.erase(fRangeList.begin() + ind);
		assert(genRange.first >= r_min);
		assert( genRange.second <= r_max);

		//update the valid range -- allow only segments with more than MAX_SUB_ROUTINE_SIZE  bytes
		if ( genRange.first - r_min > MAX_SUB_ROUTINE_SIZE+1 ) {
			fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(r_min,genRange.first-1));
		}
		if( r_max - genRange.second > MAX_SUB_ROUTINE_SIZE+1 ) {
			fRangeList.push_back(pair<sc_uint<32> , sc_uint<32> >(genRange.second+1,r_max));
		}

		sort(fRangeList.begin(),fRangeList.end());

		PrintRange(fRangeList,cerr);

		for (unsigned  int j = genRange.first ; j <=  genRange.second; j++ ) {
			set<sc_uint<32> >::iterator  it =check_set.find(j);
			if(it == check_set.end()) {
				check_set.insert(j);
			} else {
				cerr << "internal error : memory 0x" << hex << j << endl;
				assert( it == check_set.end() );
			}



		}

		if(fRangeList.size() == 0) {
			cerr << "NO MORE MEMORY"  << endl;
		}

	}


	bool GetGenRangebyLoadStore( pair<sc_uint<32>,sc_uint<32> >&  genRange, sc_uint<32>  min_size, sc_uint<32>  max_size )
	{
		if( fRangeListForLS.size() > 0 ) {
			//get index
			unsigned int ind = rand() % fRangeListForLS.size();
			return GetGenRange( genRange,min_size,min_size,ind,fRangeListForLS);
		}
		return false;
	}
	bool GetGenRangebyJump( pair<sc_uint<32>,sc_uint<32> >&  genRange, sc_uint<32>  min_size, sc_uint<32>  max_size )
	{
		if( fRangeListForFetch.size() > 0 ) {
			//get index
			unsigned int ind = rand() % fRangeListForFetch.size();

			bool st =  GetGenRange( genRange,min_size,min_size,ind,fRangeListForFetch);
			//make alignment
			if( (genRange.first %2) != 0 ) {  genRange.first = genRange.first + 1 ;}
			return st;
		}
		return false;
	}

	//generate range used by load store and branch intruction
	bool GetGenRange( pair<sc_uint<32>,sc_uint<32> >&  genRange, sc_uint<32>  min_size, sc_uint<32>  max_size , unsigned ind,vector< pair<sc_uint<32> , sc_uint<32> > > &fRangeList) {

		bool st = true;

		if( fRangeList.size() > 0 ) {

			//cerr << dec << " IND " << ind << " from MAX IND " << fRangeList.size() << endl;
			assert(ind < fRangeList.size());

			sc_uint<32>  r_left = fRangeList[ind].first;
			sc_uint<32>  r_right = fRangeList[ind].second;


			addressRangeConstraint.SetMinSize(min_size);
			addressRangeConstraint.SetMaxSize(max_size);
			//cerr << " .... from " << r_left << "====>>>" <<  r_right << endl;

			addressRangeConstraint.GenRange(r_left,r_right,genRange);

			//cerr << "... generated range " << genRange.first << ":" << genRange.second << endl;

			RemoveUpdateRange( ind,  genRange ,fRangeList);

		} else {

			cerr << "ERROR no more space in generation memory .... should exit";
			st = false;


		}

		return st;

	}

	void UnifyLSList(vector <pair<sc_uint<32>,sc_uint<32> > >& lranges) {

		sort(lranges.begin(),lranges.end());

		vector <pair<sc_uint<32>,sc_uint<32> > >::iterator it = lranges.begin();
		vector <pair<sc_uint<32>,sc_uint<32> > >::iterator it_next;


		while(it!=lranges.end()) {
			it_next=it+1;
			if(it_next==lranges.end()) break;


			if( it_next->first <= it->second) {
				if( it_next->second > it->second ) {
					it->second = it_next->second;
				}
				lranges.erase(it_next);

			} else {
				it++;
			}



		}

	}

	void AddLoadMultiUseRange( pair<sc_uint<32> , sc_uint<32>  > section) {
		cerr << "AddLoadMultiUseRange " << hex << section.first << dec <<endl;

		fLoadNotSafeRangeList.push_back(section);

	}
	void AddLoadStoreSection( pair<sc_uint<32> , sc_uint<32>  > section) {
		cerr << "AddLoadStoreSection " << hex << section.first << "--- " << section.second << dec <<endl;

		fLSFillRangeList.push_back(section);
	//	sort(fLSFillRangeList.begin(),fLSFillRangeList.end());
	}

	void PrintAssemlySections(::std::ostream& s= ::std::cout ) {

		vector <pair<sc_uint<32>,sc_uint<32> > >::iterator it;

		//cerr << "take care of multi use sections" << endl;
		for ( it  = fLoadNotSafeRangeList.begin() ; it != fLoadNotSafeRangeList.end(); it++ ) {
			fLSFillRangeList.push_back(*it);
		}

		//for ( it  = fLSFillRangeList.begin() ; it != fLSFillRangeList.end(); it++ ) {
		//	cerr << *it << endl;
		//}

		//cerr << fLSFillRangeList.size() <<endl;
		UnifyLSList(fLSFillRangeList);

		//cerr << "take care of multi use sections(2)" << endl;

		//for ( it  = fLSFillRangeList.begin() ; it != fLSFillRangeList.end(); it++ ) {
		//	cerr << *it << endl;
		//}

		for ( it  = fLSFillRangeList.begin() ; it != fLSFillRangeList.end(); it++ ) {
			s << hex << ".section LS___"<< it->first <<",\"a\",@progbits     ;  "<<dec  << endl;
			for ( unsigned  i = it->first ; i < it->second; i++) {
				sc_uint<8> bval = (sc_uint<32>(rand()))(7,0);
				if( fAccessMem.find(i) != fAccessMem.end() ) {
					bval = fAccessMem[i];
				}
				s << "\t.byte 0x" << hex << bval << "//0x" << i <<dec <<endl;
			}
		}


	}
	void PrintLinkerSections(::std::ostream& s= ::std::cout ) {
		vector <pair<sc_uint<32>,sc_uint<32> > >::iterator it;
		for ( it  = fLSFillRangeList.begin() ; it != fLSFillRangeList.end(); it++ ) {
			s << hex << "output_LS___"<< it->first << " 0x" << it->first  << ":{" << "test.o"<< "(LS___"<< it->first << ")"<< "}/*size: 0x"
					<< (it->second -   it->first)<< "*/"<<dec  << endl;
		}
	}

	//used to assign the registers
	void FillStartMemRanges(vector<sc_uint<32> > &v) {
		vector< pair<sc_uint<32> , sc_uint<32> > >::iterator it;
		for( it = fRangeListForFetch.begin() ; it  != fRangeListForFetch.end() ; it++ ) {
			v.push_back(it->first);
		}

		for( it = fRangeListForLS.begin() ; it  != fRangeListForLS.end() ; it++ ) {
			v.push_back(it->first);
		}
	}

	//Set data for Load/Store memory
	void SetMem(unsigned addr, sc_uint<8> data) {
		//cerr << "SET MEM " << hex << addr << "="<< data << dec <<endl;

		//check if data is not in array
		assert(fAccessMem.find(addr) == fAccessMem.end());

		fAccessMem[addr] = data;
	}
	//Set data for Load/Store memory
	sc_uint<8> GetMem(unsigned addr) {
		//check if data is not in array
		assert(fAccessMem.find(addr) != fAccessMem.end());

		//cerr << hex << "get mem addr[" << addr << "]=" <<  fAccessMem[addr]  << dec << endl;

		return ( fAccessMem[addr] );
	}
private:

	SAddressRangeConstraint addressRangeConstraint;


private:
	vector< pair<sc_uint<32> , sc_uint<32> > > fRangeListForLS; // initial memory designated for load/store
	vector< pair<sc_uint<32> , sc_uint<32> > > fRangeListForFetch; // initial memory designated for fetch

	vector< pair<sc_uint<32> , sc_uint<32> > > fLSFillRangeList; // only load store -- exclusive access no double read or write to the same place
	vector< pair<sc_uint<32> , sc_uint<32> > > fLoadNotSafeRangeList; // can read from write or fetch memory

	//for validation purpose only
	set<sc_uint<32> > check_set;

};

#endif
