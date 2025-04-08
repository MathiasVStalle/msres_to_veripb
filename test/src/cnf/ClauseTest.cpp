#include "../../test.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <cassert>
#include <set>

#include "../../../src/cnf/Clause.h"

void construct_clause_test() {
    std::set<int32_t> literals = {1, -2, 3};
    cnf::Clause clause(literals);
    
    assert(clause.getWeight() == 0);
    assert(clause.getLiterals() == literals);

    std::set<int32_t> literals2 = {-1, 2, -3};
    cnf::Clause clause2(5, literals2);

    assert(clause2.getWeight() == 5);
    assert(clause2.getLiterals() == literals2);
}

void get_weight_test() {
    std::set<int32_t> literals = {1, -2, 3};
    cnf::Clause clause(literals);
    
    assert(clause.getWeight() == 0);

    std::set<int32_t> literals2 = {-1, 2, -3};
    cnf::Clause clause2(5, literals2);

    assert(clause2.getWeight() == 5);
}

void get_literals_test() {
    std::set<int32_t> literals = {1, -2, 3};
    cnf::Clause clause(literals);
    
    assert(clause.getLiterals() == literals);

    std::set<int32_t> literals2 = {-1, 2, -3};
    cnf::Clause clause2(5, literals2);

    assert(clause2.getLiterals() == literals2); 
}

void copy_clause_test() {
    std::set<int32_t> literals = {1, -2, 3};
    cnf::Clause clause(literals);
    
    cnf::Clause clause_copy(clause);
    
    assert(clause_copy.getWeight() == clause.getWeight());
    assert(clause_copy.getLiterals() == clause.getLiterals());
}



test_func clause_tests[] = {
    construct_clause_test,
    copy_clause_test,
    get_weight_test,
    get_literals_test,
};

const char *clause_test_names[] = {
    "construct_clause_test",
    "copy_clause_test",
    "get_weight_test",
    "get_literals_test",
};

void clause_test()
{
    int amount_of_tests = sizeof(clause_tests) / sizeof(test_func);
    int passed_tests = 0;

    for (int i = 0; i < amount_of_tests; i++)
    {
        int pid = fork();
        if (pid == 0)
        {
            std:: cout << "Running " << clause_test_names[i] << std::endl;
            clause_tests[i]();
            exit(0);
        }

        int status;
        wait(&status);
        if (status == 0)
        {
            std::cout << GREEN << "Test " << clause_test_names[i] << " passed" << RESET << std::endl;
            passed_tests++;
        }
        else
        {
            std::cout << RED << "Test " << clause_test_names[i] << " failed" << RESET << std::endl;
        }
    }

    std::cout << "Test passed: " << passed_tests << "/" << amount_of_tests << std::endl;
}