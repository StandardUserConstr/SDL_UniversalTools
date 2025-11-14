
#include <string.h>
#include <new>
#include <utility>
//#include <assert.h>

// #include <string.h>  only for memcpy();
// #include <new>       for "new"
// #include <utility>   for "std::move()" and others functions
// #include <assert.h>  for "assert()" only (you can not include this if you're not ussing "assert()" in [] operator);
// this class doesn't support types of T whose throws exceptions;
// the class may contain bugs and should be tested even more strongly in every possible way;
// if you have compilator error for these linse: if (std::is_trivially_copyable<T>::value) then off warnings or make these lines:
//      if constexpr (std::is_trivially_copyable<T>::value) and add compilation option: -std=c++17; it will calculate these lines at compilation time;
//      or just off warning by adding option to compilator clang/GNU: "-Wno-class-memaccess" or "/wd4996" in MSVC;
// remember that if real_size_in_bytes() returns number divided by 16 (16-32-64 and so on) then u support technologies such as generating SSE2 instructions
//      in optymalized assembly code which speeds up some functions in this class like "push_back()";
//
// this vector has especially push_back faster propably for about ~2-6x (and can be far faster if u change multiplication/elements_count_start for cost of ram)
//      comparing to std::vector and some other functions/instructions that are faster or comparablely fast to functions of std::vector;
//      but has slower constructor for about ~x4 because of initial allocating memory in constructor for optymalization reasons of "push_back()";
//      and std::vector doesn't allocate memory at start and its capacity is 0 while my CustomVector must allocate at least 1 element at the start;
//      but constructor isn't expensive anyway and isn't done often so that's it;
//      good option for those who are using a lot push_back with huge classes to move/copy objects to vector;
//  Casual benchmarks:
//      Test environment:
//          Compilator: g++ (tdm64-1) 10.3.0;
//          Processor: intel core duo 2x2ghz;
//          Everything tested with CustomVector initial options: "2 multiplication" and "4 elements_count_start"
//              so it's close enough to the original std::vector);
//      the benchmark shouldn't be taken into account too much because the tests could not be performed in the fully revealing
//          performance as to the percentage but the general image is certainly correct;
//
//  push_back() in CustomVector compared to std::vector is:
//      ~389% faster with: | move semantics (std::move) | 16bytes class | no optymalization
//      ~643% faster with: | move semantics (std::move) | 16bytes class | optymalization -O2
//          (results with -O2 are also similar to optymlization with -O1 and -O3 (~650%<->~800%) so i skiped it to write this down;
//              if i sees something interesting with the next tests when it comes to optimizations i will show results with other flags);
//      ~244% faster with: | copying semantics (object as argument) | 16bytes class | no optymalization
//      ~270% faster with: | copying semantics (object as argument) | 16bytes class | optymalization -O2
//      ~183% faster with: | moving semantics (std::move) | 4bytes int | no optymalization
//      ~113% faster with: | moving semantics (std::move) | 4bytes int | optymalization -O2
//      ~206% faster with: | moving semantics (number as argument) | 4bytes int | no optymalization
//      ~166% faster with: | moving semantics (number as argument) | 4bytes int | optymalization -02
//      ~211% faster with: | copying semantics (variable as argument) | 4bytes int | no optymalization
//      ~269% faster with: | copying semantics (variable as argument) | 4bytes int | optymalization -02
//  reserve() in CustomVector compared to std::vector is:
//      ~26% faster with: | 4bytes int | no optymalization
//      ~4% slower with: | 4bytes int | optymalization -02  (sometimes faster,sometimes slower);
//  shrink_to_fit() in CustomVector compared to std::vector is:
//      ~1040% faster with: | 16bytes class | no optymalization
//      ~5850% faster with: | 16bytes class | optymalization -02
//      ~14% faster with: | 4bytes int | no optymalization
//      ~23% slower with: | 4bytes int | optymalization -02
//  using move/copy operators between CustomVector's is also a lot faster;
//  creating constructor is of course ~4x times slower than in std::vector and clear() is also slower than in std::vector
//      but because my functions do different things so i'm not even going to compare them 'cause this is pointless;
//  everyithing else should has basically the same speed as the original std::vector;
template <typename T> class CustomVector
{

#if defined(__clang__)
  #define PUSH_WARNING_IGNORE_Wclass_memaaccess() \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wclass-memaccess\"")
  #define POP_WARNING() \
    _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
  #define PUSH_WARNING_IGNORE_Wclass_memaaccess() \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wclass-memaccess\"")
  #define POP_WARNING() \
    _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
  #define PUSH_WARNING_IGNORE_Wclass_memaaccess() \
    __pragma(warning(push)) \
    __pragma(warning(disable: 4996))
  #define POP_WARNING() \
    __pragma(warning(pop))
#else
  #define PUSH_WARNING_IGNORE()
  #define POP_WARNING()
#endif

    T* main_data;
    size_t element_count;
    size_t size_of_objects;
    size_t max_elements;
    size_t multiplication;
    size_t elements_count_start;

public:

    CustomVector(size_t elements_count_start = 4,size_t multiplication = 2)       //  multiplication: higher value == better optimalization for "push_back" function but higher usage of ram;
    {
        if(elements_count_start==0) this->elements_count_start = 1;
        else this->elements_count_start = elements_count_start;
        if(multiplication<2) this->multiplication = 2;
        else this->multiplication = multiplication;

        this->main_data = (T*)operator new(sizeof(T)*this->elements_count_start);
        this->element_count = 0;
        this->size_of_objects = sizeof(T)*this->elements_count_start;
        this->max_elements = this->elements_count_start;
        return;
    }

// clear() deallocate memory and shrinks it to the default size of the CustomVector;
    void clear() noexcept
    {
        if(this->main_data!=NULL)
        {
            if(!std::is_trivially_copyable<T>::value) // optymalization for variables like int (not classes with constructors);
            {
                for (size_t i = 0; i!=this->element_count; i++) this->main_data[i].~T();  // even if 'T' isn't class then there's is no problem; compilator will allow this and
                                                                                                //  do nothing about it; compilator just skips it in that case;
            }

            operator delete(this->main_data);       // "operator delete" and "delete" are 2 different things; "operator delete" doesn't run destructor;
        }

        this->element_count = 0;
        this->size_of_objects = sizeof(T)*this->elements_count_start;
        this->main_data = (T*)operator new(sizeof(T)*this->elements_count_start);
        this->max_elements = this->elements_count_start;
        return;
    }

    void push_back(const T& variable) noexcept
    {
        if(this->element_count==this->max_elements)
        {
            this->size_of_objects = sizeof(T)*this->max_elements*this->multiplication;

            T* copy_data = (T*)operator new(this->size_of_objects);
            if(std::is_trivially_copyable<T>::value)   // optymalization for variables like int (not classes with constructors);
            {
                PUSH_WARNING_IGNORE_Wclass_memaaccess();
                memcpy(copy_data,this->main_data,sizeof(T)*this->element_count);
                POP_WARNING();
            }
            else
            {
                for (size_t i = 0; i!=element_count; i++)
                {
                    new (&copy_data[i]) T(std::move(this->main_data[i]));
                    this->main_data[i].~T();
                }
            }
            operator delete(this->main_data);

            this->main_data = copy_data;
            this->max_elements = this->max_elements*this->multiplication;
        }
        if(std::is_trivially_copyable<T>::value)   // optymalization for variables like int (not classes with constructors);
        {
            this->main_data[this->element_count] = variable;
        }
        else new (&this->main_data[this->element_count]) T(variable);
        this->element_count++;
        return;
    }

    void push_back(T&& variable) noexcept
    {
        if(this->element_count==this->max_elements)
        {
            this->size_of_objects = sizeof(T)*this->max_elements*this->multiplication;

            T* copy_data = (T*)operator new(this->size_of_objects);
            if(std::is_trivially_copyable<T>::value)   // optymalization for variables like int (not classes with constructors);
            {
                PUSH_WARNING_IGNORE_Wclass_memaaccess();
                memcpy(copy_data,this->main_data,sizeof(T)*this->element_count);
                POP_WARNING();
            }
            else
            {
                for (size_t i = 0; i!=element_count; i++)
                {
                    new (&copy_data[i]) T(std::move(this->main_data[i]));
                    this->main_data[i].~T();
                }
            }
            operator delete(this->main_data);

            this->main_data = copy_data;
            this->max_elements = this->max_elements*this->multiplication;
        }
        if(std::is_trivially_copyable<T>::value)   // optymalization for variables like int (not classes with constructors);
        {
            this->main_data[this->element_count] = variable;
        }
        else new (&this->main_data[this->element_count]) T(std::move(variable));
        this->element_count++;
        return;
    }

    void pop_back() noexcept
    {
        if(this->element_count!=0)
        {
            this->element_count--;
            this->main_data[this->element_count].~T();
        }
        return;
    }

    void erase(size_t first,size_t last) noexcept
    {
        if(this->element_count==0) return;
        if(last>=this->element_count) last = this->element_count-1;
        if(first>last) return;

        for(size_t i = first; i<=last; i++) this->main_data[i].~T();

        if(std::is_trivially_copyable<T>::value)   // optymalization for variables like int (not classes with constructors);
        {
            for(size_t i = last+1,j = first; i<this->element_count; i++,j++)
            {
                this->main_data[j] = this->main_data[i];
            }
        }
        else
        {
            for(size_t i = last+1,j = first; i<this->element_count; i++,j++)
            {
                new (&this->main_data[j]) T(std::move(this->main_data[i]));
                this->main_data[i].~T();
            }
        }

        this->element_count-=(last-first)+1;

        return;
    }

    void reserve(size_t elements) noexcept
    {
        if (elements==0) elements = 1;
        if(elements<=this->max_elements) return;

        this->size_of_objects = sizeof(T)*elements;

        T* copy_data = (T*)operator new(this->size_of_objects);
        if(std::is_trivially_copyable<T>::value)   // optymalization for variables like int (not classes with constructors);
        {
            PUSH_WARNING_IGNORE_Wclass_memaaccess();
            memcpy(copy_data,this->main_data,sizeof(T)*this->element_count);
            POP_WARNING();
        }
        else
        {
            for (size_t i = 0; i!=this->element_count; i++)
            {
                new (&copy_data[i]) T(std::move(this->main_data[i]));
                this->main_data[i].~T();
            }
        }
        operator delete(this->main_data);

        this->main_data = copy_data;
        this->max_elements = elements;
        return;
    }

// if "element_count" is 0,then function forces minimum 1 element to be allocated in this class;
    void shrink_to_fit() noexcept
    {
        if(this->element_count==this->max_elements) return;

        size_t forced_miminum_1_element_allocate = 0;
        if(this->element_count==0) forced_miminum_1_element_allocate = 1;

        this->size_of_objects = sizeof(T)*(this->element_count+forced_miminum_1_element_allocate);

        T* copy_data = (T*)operator new(this->size_of_objects);
        if (std::is_trivially_copyable<T>::value)   // optymalization for variables like int (not classes with constructors);
        {
            PUSH_WARNING_IGNORE_Wclass_memaaccess();
            memcpy(copy_data,this->main_data,sizeof(T)*this->element_count);
            POP_WARNING();
        }
        else
        {
            for (size_t i = 0; i!=this->element_count; i++)
            {
                new (&copy_data[i]) T(std::move(this->main_data[i]));
                this->main_data[i].~T();
            }
        }
        operator delete(this->main_data);

        this->main_data = copy_data;
        this->max_elements = this->element_count+forced_miminum_1_element_allocate;

        return;
    }

// changes multiplication of growing array when using function "push_back()";
    void change_multiplication(size_t new_multiplication)
    {
        if(new_multiplication<2) this->multiplication = 2;
        else this->multiplication = new_multiplication;
        return;
    }

// changes starting reserve size for elements; used by function "clear()":
    void change_elements_count_start(size_t new_elements_count_start)
    {
        if(new_elements_count_start==0) this->elements_count_start = 1;
        else this->elements_count_start = new_elements_count_start;
        return;
    }

    size_t size() const
    {
        return this->element_count;
    }

    size_t real_size_in_bytes() const
    {
        return this->size_of_objects;
    }

    size_t size_of_elements_in_bytes() const
    {
        return this->element_count*sizeof(T);
    }

    bool empty() const
    {
        return 1 ? this->element_count == 0 : 0;
    }

    const T& operator[](const size_t iterator) const
    {
        //assert(iterator<this->element_count);           // can be uncommented in debugging version of the program but performance will be lower;
        return this->main_data[iterator];
    }

    T& operator[](const size_t iterator)
    {
        //assert(iterator<this->element_count);           // can be uncommented in debugging version of the program but performance will be lower;
        return this->main_data[iterator];
    }


    ~CustomVector() noexcept
    {
        if(this->main_data!=NULL)
        {
            if(!std::is_trivially_copyable<T>::value)
            {
                for (size_t i = 0; i!=this->element_count; i++) this->main_data[i].~T();
            }
            operator delete(this->main_data);
            this->main_data = NULL;
        }
        return;
    }

//  implementation for "rule of the five" needed 'cause of manual memory management existing in class (new/malloc);
//------------------------------------------------------------------------------------------------------------------------------

//  copying constructor; used like:
//      Class object0 = object1;
//      Class object0 = return_object();
//          [inside return_object]:
//          return object2;
    CustomVector(const CustomVector& other)
    {
        this->multiplication = other.multiplication;
        this->elements_count_start = other.elements_count_start;
        this->element_count = other.element_count;
        this->max_elements = other.max_elements;
        this->size_of_objects = other.size_of_objects;
        this->main_data = (T*)operator new(this->size_of_objects);


        if(std::is_trivially_copyable<T>::value)
        {
            PUSH_WARNING_IGNORE_Wclass_memaaccess();
            memcpy(this->main_data,other.main_data,sizeof(T)*this->element_count);
            POP_WARNING();
        }
        else
        {
            for (size_t i = 0; i!=this->element_count; i++) new (&this->main_data[i]) T(other.main_data[i]);
        }

        return;
    }

//  copying operator; used like:
//      object0 = object1;
//      object0 = return_object();
//          [inside return_object]:
//          return object2;
    CustomVector& operator=(const CustomVector& other)
    {
        if (this==&other) return *this;   // check if you're using operator '=' with the same object;

        if(other.element_count>this->max_elements)  //  optymalization;
        {
            if(this->main_data!=NULL)
            {
                if(!std::is_trivially_copyable<T>::value)
                {
                    for (size_t i = 0; i!=this->element_count; i++) this->main_data[i].~T();
                }
                operator delete(this->main_data);
            }

            // this->multiplication = other.multiplication;             // i want multiplication to be set as original has been setted;
            //this->elements_count_start = other.elements_count_start;  // i want elements_count_start to be set as original has been setted;
            this->element_count = other.element_count;
            this->max_elements = other.max_elements;
            this->size_of_objects = other.size_of_objects;
            this->main_data = (T*)operator new(this->size_of_objects);

            if(std::is_trivially_copyable<T>::value)
            {
                PUSH_WARNING_IGNORE_Wclass_memaaccess();
                memcpy(this->main_data,other.main_data,sizeof(T)*this->element_count);
                POP_WARNING();
            }
            else
            {
                for (size_t i = 0; i!=this->element_count; i++) new (&this->main_data[i]) T(other.main_data[i]);
            }

        }
        else    // main_data is large enough;
        {
            if(!std::is_trivially_copyable<T>::value)
            {
                for(size_t i = other.element_count; i<this->element_count; i++) this->main_data[i].~T();

                for(size_t i = 0; i!=other.element_count; i++)
                {
                    if (i<this->element_count) this->main_data[i] = other.main_data[i];
                    else new (&this->main_data[i]) T(other.main_data[i]);
                }
            }
            else
            {
                PUSH_WARNING_IGNORE_Wclass_memaaccess();
                memcpy(this->main_data,other.main_data,sizeof(T)*other.element_count);
                POP_WARNING();
            }

            this->element_count = other.element_count;
        }

        return *this;
    }

//  moving constructor; used like:
//      Class object0 = Class();
//      Class object0 = std::move(object1);
//      Class object0 = return_object();
//          [inside return_object]:
//          return Class();
    CustomVector(CustomVector&& other) noexcept   // noexcept is important for new c++ gadgets to inform that function doesn't throw exception
    {                                                //     so function that doesn't support exception can run this function except copying function;
        this->main_data = other.main_data;
        this->element_count = other.element_count;
        this->size_of_objects = other.size_of_objects;
        this->max_elements = other.max_elements;
        this->multiplication = other.multiplication;
        this->elements_count_start = other.elements_count_start;

        other.main_data = NULL;
        other.element_count = 0;
        other.size_of_objects = 0;
        other.max_elements = 0;

        return;
    }

//  moving operator; used like:
//      object0 = Class();
//      object0 = std::move(object1);
//      object0 = return_object();
//          [inside return_object]:
//          return Class();
    CustomVector& operator=(CustomVector&& other) noexcept // noexcept is important for new c++ gadgets to inform that function doesn't throw exception
    {                                                          //     so function that doesn't support exception can run this function except copying function;
        if (this==&other) return *this;     // check if you're using operator '=' with the same object;

        if (this->main_data!=NULL)
        {
            if(!std::is_trivially_copyable<T>::value)
            {
                for (size_t i = 0; i!=this->element_count; i++) this->main_data[i].~T();
            }
            operator delete(this->main_data);
        }

        this->main_data = other.main_data;
        this->element_count = other.element_count;
        this->size_of_objects = other.size_of_objects;
        this->max_elements = other.max_elements;
        //this->multiplication = other.multiplication;
        //this->elements_count_start = other.elements_count_start;

        other.main_data = NULL;
        other.element_count = 0;
        other.size_of_objects = 0;
        other.max_elements = 0;

        return *this;
    }
//------------------------------------------------------------------------------------------------------------------------------

};