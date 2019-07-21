//
// Created by Matthias Preymann on 21.07.2019.
//

#ifndef PROMISE_LAMBDACONTAINER_H
#define PROMISE_LAMBDACONTAINER_H


#include <utility>

/**
 * Templated Lambda Container Class
 * Holds a callable object (functor) of arbitrary type by value
 *
 * @tparam T_Lambda - type of functor to be stored
 */
template< typename T_Lambda >
class LambdaContainer {
protected:
    T_Lambda m_lambda;

public:
    LambdaContainer( T_Lambda&& lam )
            : m_lambda( std::forward<T_Lambda>(lam) ) {}

    inline T_Lambda& get() { return m_lambda; }
};


#endif //PROMISE_LAMBDACONTAINER_H
