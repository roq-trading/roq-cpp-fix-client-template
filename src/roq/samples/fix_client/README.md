

```bash
$ curl http://localhost:1234/metrics
# TYPE roq_request_latency histogram
roq_request_latency_bucket{source="fix-proxy", function="internal", le="10000"} 0
roq_request_latency_bucket{source="fix-proxy", function="internal", le="100000"} 0
roq_request_latency_bucket{source="fix-proxy", function="internal", le="1000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="internal", le="10000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="internal", le="100000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="internal", le="1000000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="internal", le="+Inf"} 0
roq_request_latency_sum{source="fix-proxy", function="internal"} 0
roq_request_latency_count{source="fix-proxy", function="internal"} 0

# TYPE roq_request_latency histogram
roq_request_latency_bucket{source="fix-proxy", function="external", le="10000"} 0
roq_request_latency_bucket{source="fix-proxy", function="external", le="100000"} 0
roq_request_latency_bucket{source="fix-proxy", function="external", le="1000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="external", le="10000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="external", le="100000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="external", le="1000000000"} 0
roq_request_latency_bucket{source="fix-proxy", function="external", le="+Inf"} 0
roq_request_latency_sum{source="fix-proxy", function="external"} 0
roq_request_latency_count{source="fix-proxy", function="external"} 0
```
