library(ggplot2)
library(dplyr)
library(tibble)
library(readr)
library(tidyr)
runtime_data_tmp <- read.csv("Desktop/TaskMiner/scripts/R_scripts/data.csv")
runtime_data <- runtime_data_tmp[order(runtime_data_tmp$Speedup),]
indexS <- paste("S", seq(1,5), sep = "")
indexP <- paste("P", seq(1,5), sep = "")

BENCH <- runtime_data[,1]
BENCH <- cbind(BENCH,log(runtime_data[,indexP] / runtime_data[,indexS]))
metric = gather(BENCH, key='id', value='value', 2, 3, 4, 5, 6)
agg1 <- aggregate(value ~ BENCH, metric, function (x) c(Mean=mean(x), Sd=sd(x), Min=min(x), Max=max(x)))
agg1 <- cbind(agg1, runtime_data[,"Speedup"])
agg1$value = as.data.frame(agg1$value)
colnames(agg1)[length(colnames(agg1))] <- "Speedup"
p = ggplot() + geom_boxplot(data=agg1, aes(x = BENCH, ymin = value$Min, ymax = value$Max, lower = Speedup - value$Sd, upper = Speedup + value$Sd, middle = Speedup), stat = "identity") + coord_flip()  +
  theme(axis.text.y = element_text(lineheight = 0.5
                                   , size = 6))
ggsave("boxplot.pdf", path="Desktop/TaskMiner/scripts/R_scripts", plot = p, device="pdf", limitsize = FALSE)

                  
                  
