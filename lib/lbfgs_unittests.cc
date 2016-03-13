#include "lbfgs.h"

#include <gtest/gtest.h>
#include <cstdio>
#include <cmath>

namespace {
  class objective_function
  {
  protected:
    lbfgsfloatval_t *m_x;

  public:
    objective_function() : m_x(NULL)
    {
    }

    virtual ~objective_function()
    {
      if (m_x != NULL) {
        lbfgs_free(m_x);
        m_x = NULL;
      }
    }

    lbfgsfloatval_t* getResults() {
      return m_x;
    }

    int run(int N)
    {
      lbfgsfloatval_t fx;
      m_x = lbfgs_malloc(N);

      if (m_x == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate a memory block for variables.\n");
        return 1;
      }

      /* Initialize the variables. */
      for (int i = 0;i < N;i += 2) {
        m_x[i] = -1.2;
        m_x[i+1] = 1.0;
      }

      /*
        Start the L-BFGS optimization; this will invoke the callback functions
        evaluate() and progress() when necessary.
      */
      int ret = lbfgs(N, m_x, &fx, &_evaluate, &_progress, this, NULL);

      /* Report the result. */
      printf("L-BFGS optimization terminated with status code = %d\n", ret);
      printf("  fx = %f, x[0] = %f, x[1] = %f\n", fx, m_x[0], m_x[1]);

      return ret;
    }

  protected:
    static lbfgsfloatval_t _evaluate(
                                     void *instance,
                                     const lbfgsfloatval_t *x,
                                     lbfgsfloatval_t *g,
                                     const int n,
                                     const lbfgsfloatval_t step
                                     )
    {
      return reinterpret_cast<objective_function*>(instance)->evaluate(x, g, n, step);
    }

    lbfgsfloatval_t evaluate(
                             const lbfgsfloatval_t *x,
                             lbfgsfloatval_t *g,
                             const int n,
                             const lbfgsfloatval_t step
                             )
    {
      lbfgsfloatval_t fx = 0.0;

      for (int i = 0;i < n;i += 2) {
        lbfgsfloatval_t t1 = 1.0 - x[i];
        lbfgsfloatval_t t2 = 10.0 * (x[i+1] - x[i] * x[i]);
        g[i+1] = 20.0 * t2;
        g[i] = -2.0 * (x[i] * g[i+1] + t1);
        fx += t1 * t1 + t2 * t2;
      }
      return fx;
    }

    static int _progress(
                         void *instance,
                         const lbfgsfloatval_t *x,
                         const lbfgsfloatval_t *g,
                         const lbfgsfloatval_t fx,
                         const lbfgsfloatval_t xnorm,
                         const lbfgsfloatval_t gnorm,
                         const lbfgsfloatval_t step,
                         int n,
                         int k,
                         int ls
                         )
    {
      return reinterpret_cast<objective_function*>(instance)->progress(x, g, fx, xnorm, gnorm, step, n, k, ls);
    }

    int progress(
                 const lbfgsfloatval_t *x,
                 const lbfgsfloatval_t *g,
                 const lbfgsfloatval_t fx,
                 const lbfgsfloatval_t xnorm,
                 const lbfgsfloatval_t gnorm,
                 const lbfgsfloatval_t step,
                 int n,
                 int k,
                 int ls
                 )
    {
      printf("Iteration %d:\n", k);
      printf("  fx = %f, x[0] = %f, x[1] = %f\n", fx, x[0], x[1]);
      printf("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
      printf("\n");
      return 0;
    }
  };

}

class LbfgsTest : public ::testing::Test {
protected:
  static double const EPSILON = 1e-6;

  objective_function obj;
};
double const LbfgsTest::EPSILON;

TEST_F(LbfgsTest, testObjectiveFunction) {
  int const N = 100;
  int rc = obj.run(N);
  ASSERT_EQ(0, rc);
  lbfgsfloatval_t* results = obj.getResults();
  ASSERT_NE((lbfgsfloatval_t*) NULL, results);
  EXPECT_LE(abs(results[0] - 1.0), EPSILON) << results[0] << " not within " << EPSILON << " of 1";
  EXPECT_LE(abs(results[1] - 1.0), EPSILON) << results[1] << " not within " << EPSILON << " of 1";
}
